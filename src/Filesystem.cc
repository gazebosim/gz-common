/*
 * Copyright (C) 2016 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#include <sys/stat.h>

#ifdef __linux__
#include <sys/sendfile.h>
#endif

#include <iomanip>
#include <array>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <regex>

#include <gz/common/config.hh>
#include <gz/common/SystemPaths.hh>
#include <gz/common/Util.hh>
#include <gz/common/Uuid.hh>
#include <gz/common/Console.hh>

#ifndef _WIN32
#include <dirent.h>
#include <limits.h>
#include <climits>
#else
#include <shlwapi.h>
#include <io.h>
#include "win_dirent.h"
#include "PrintWindowsSystemWarning.hh"
#endif

#include "gz/common/Filesystem.hh"

#ifdef _WIN32
# define IGN_PATH_MAX _MAX_PATH
#elif defined(PATH_MAX)
# define IGN_PATH_MAX PATH_MAX
#elif defined(_XOPEN_PATH_MAX)
# define IGN_PATH_MAX _XOPEN_PATH_MAX
#else
# define IGN_PATH_MAX _POSIX_PATH_MAX
#endif

namespace igncmn = gz::common;
using namespace ignition;
using namespace igncmn;

/////////////////////////////////////////////////
bool common::isFile(const std::string &_path)
{
  std::ifstream f(_path);
  return (!isDirectory(_path)) && f.good();
}

/////////////////////////////////////////////////
bool common::removeDirectory(const std::string &_path,
                                       const FilesystemWarningOp _warningOp)
{
  bool removed = false;
  if (common::isDirectory(_path))
  {
#ifdef _WIN32
    removed = RemoveDirectory(_path.c_str());
    if (!removed && FSWO_LOG_WARNINGS == _warningOp)
    {
      common::PrintWindowsSystemWarning(
            "Failed to remove directory [" + _path + "]");
    }
#else
    removed = (rmdir(_path.c_str()) == 0);
    if (!removed)
    {
      // A sym link would end up here
      removed = (std::remove(_path.c_str()) == 0);
    }

    if (!removed && FSWO_LOG_WARNINGS == _warningOp)
    {
      ignwarn << "Failed to remove directory [" + _path + "]: "
              << std::strerror(errno) << "\n";
    }
#endif
  }
  else if (_warningOp)
  {
    ignwarn << "The path [" << _path << "] does not refer to a directory\n";
  }

  return removed;
}

/////////////////////////////////////////////////
bool common::removeFile(const std::string &_existingFilename,
                                  const FilesystemWarningOp _warningOp)
{
  const bool removed = (std::remove(_existingFilename.c_str()) == 0);
  if (!removed && FSWO_LOG_WARNINGS == _warningOp)
  {
    ignwarn << "Failed to remove file [" << _existingFilename << "]: "
            << std::strerror(errno) << "\n";
  }

  return removed;
}

/////////////////////////////////////////////////
bool common::removeDirectoryOrFile(
    const std::string &_path,
    const FilesystemWarningOp _warningOp)
{
  if (common::isDirectory(_path))
  {
    return common::removeDirectory(_path, _warningOp);
  }
  else if (common::isFile(_path))
  {
    return common::removeFile(_path, _warningOp);
  }
  else if (FSWO_LOG_WARNINGS == _warningOp)
  {
    ignwarn << "The path [" << _path << "] does not refer to a "
            << "directory nor to a file\n";
  }
  return false;
}

/////////////////////////////////////////////////
bool common::removeAll(const std::string &_path,
                                 const FilesystemWarningOp _warningOp)
{
  if (common::isDirectory(_path))
  {
    DIR *dir = opendir(_path.c_str());
    if (dir)
    {
      struct dirent *p;
      while ((p=readdir(dir)))
      {
        // Skip special files.
        if (!std::strcmp(p->d_name, ".") || !std::strcmp(p->d_name, ".."))
          continue;

        const auto removed = common::removeAll(
          common::joinPaths(_path, p->d_name), _warningOp);
        if (!removed)
          return false;
      }
    }
    closedir(dir);
  }

  return common::removeDirectoryOrFile(_path, _warningOp);
}

/////////////////////////////////////////////////
bool common::moveFile(const std::string &_existingFilename,
                                const std::string &_newFilename,
                                const FilesystemWarningOp _warningOp)
{
  if (!copyFile(_existingFilename, _newFilename, _warningOp))
    return false;

  if (removeFile(_existingFilename, _warningOp))
    return true;

  // The original file could not be removed, which means we are not
  // able to "move" it (we can only copy it, apparently). Since this
  // function is meant to move files, and we have failed to move the
  // file, we should remove the copy that we made earlier.
  removeFile(_newFilename, _warningOp);

  return false;
}

/////////////////////////////////////////////////
std::string common::absPath(const std::string &_path)
{
  std::string result;

  // cppcheck-suppress ConfigurationNotChecked
  char path[IGN_PATH_MAX] = "";
#ifdef _WIN32
  if (GetFullPathName(_path.c_str(), IGN_PATH_MAX, &path[0], nullptr) != 0)
#else
  if (realpath(_path.c_str(), &path[0]) != nullptr)
#endif
    result = path;
  else if (!_path.empty())
  {
    // If _path is an absolute path, then return _path.
    // An absolute path on Windows is a character followed by a colon and a
    // backslash.
    if (_path.compare(0, 1, "/") == 0 || _path.compare(1, 3, ":\\") == 0)
      result = _path;
    // Otherwise return the current working directory with _path appended.
    else
      result = joinPaths(common::cwd(), _path);
  }

  common::replaceAll(result, result, "//", "/");

  return result;
}

// This is help function to handle windows paths, there are a mix between '/'
// and backslashes.
// joinPaths uses the system separator, in Windows this generate some issues
// with URIs
std::string checkWindowsPath(const std::string _path)
{
  if (_path.empty())
    return _path;

  // Check if this is a http or https, if so change backslashes generated by
  // jointPaths to '/'
  if ((_path.size() > 7 && 0 == _path.compare(0, 7, "http://")) ||
      (_path.size() > 8 && 0 == _path.compare(0, 8, "https://")))
  {
    return std::regex_replace(_path, std::regex(R"(\\)"), "/");
  }

  // This is a Windows path, convert all '/' into backslashes
  std::string result = std::regex_replace(_path, std::regex(R"(/)"), "\\");
  std::string drive_letters;

  // only Windows contains absolute paths starting with drive letters
  if (result.length() > 3 && 0 == result.compare(1, 2, ":\\"))
  {
    drive_letters = result.substr(0, 3);
    result = result.substr(3);
  }
  result = drive_letters + std::regex_replace(
    result, std::regex("[<>:\"|?*]"), "");
  return result;
}

//////////////////////////////////////////////////
std::string common::joinPaths(const std::string &_path1,
                                        const std::string &_path2)
{

  /// This function is used to avoid duplicated path separators at the
  /// beginning/end of the string, and between the two paths being joined.
  /// \param[in] _path This is the string to sanitize.
  /// \param[in] _stripLeading True if the leading separator should be
  /// removed.
  auto sanitizeSlashes = [](const std::string &_path,
                            bool _stripLeading = false)
  {
    // Shortcut
    if (_path.empty())
      return _path;

    std::string result = _path;

    // Use the appropriate character for each platform.
#ifndef _WIN32
    char replacement = '/';
#else
    char replacement = '\\';
#endif

    // Sanitize the start of the path.
    size_t index = 0;
    size_t leadingIndex = _stripLeading ? 0 : 1;
    for (; index < result.length() && result[index] == replacement; ++index)
    {
    }
    if (index > leadingIndex)
      result.erase(leadingIndex, index-leadingIndex);

    // Sanitize the end of the path.
    index = result.length()-1;
    for (; index <  result.length() && result[index] == replacement; --index)
    {
    }
    index += 1;
    if (index < result.length()-1)
        result.erase(index+1);
    return result;
  };

  std::string path;
#ifndef _WIN32
  path = sanitizeSlashes(sanitizeSlashes(separator(_path1)) +
      sanitizeSlashes(_path2, true));
#else  // _WIN32
  std::string path1 = sanitizeSlashes(checkWindowsPath(_path1));
  std::string path2 = sanitizeSlashes(checkWindowsPath(_path2), true);
  std::vector<CHAR> combined(path1.length() + path2.length() + 2);
  if (::PathCombineA(combined.data(), path1.c_str(), path2.c_str()) != NULL)
  {
    path = sanitizeSlashes(checkWindowsPath(std::string(combined.data())));
  }
  else
  {
    path = sanitizeSlashes(checkWindowsPath(separator(path1) + path2));
  }
#endif  // _WIN32
  return path;
}

/////////////////////////////////////////////////
std::string common::parentPath(const std::string &_path)
{
  std::string result;

  size_t last_sep = _path.find_last_of(separator(""));
  // If slash is the last character, find its parent directory
  if (last_sep == _path.length() - 1)
    last_sep = _path.substr(0, last_sep).find_last_of(separator(""));

  result = _path.substr(0, last_sep);

  return result;
}

/////////////////////////////////////////////////
bool common::copyFile(const std::string &_existingFilename,
                                const std::string &_newFilename,
                                const FilesystemWarningOp _warningOp)
{
  std::string absExistingFilename =
    common::absPath(_existingFilename);
  std::string absNewFilename = common::absPath(_newFilename);

  if (absExistingFilename == absNewFilename)
    return false;

#ifdef _WIN32
  const bool copied = CopyFile(absExistingFilename.c_str(),
                               absNewFilename.c_str(), false);

  if (!copied && FSWO_LOG_WARNINGS == _warningOp)
  {
    common::PrintWindowsSystemWarning(
      "Failed to copy file [" + absExistingFilename
      + "] to [" + absNewFilename + "]");
  }

  return copied;
#else
  bool result = false;
  std::ifstream in(absExistingFilename.c_str(), std::ifstream::binary);

  if (in.good())
  {
    std::ofstream out(absNewFilename.c_str(),
                      std::ifstream::trunc | std::ifstream::binary);
    if (out.good())
    {
      out << in.rdbuf();
      result = common::isFile(absNewFilename);
    }
    else if (FSWO_LOG_WARNINGS == _warningOp)
    {
      ignwarn << "Failed to create file [" << absNewFilename << "]: "
              << std::strerror(errno) << "\n";
    }
    out.close();
  }
  else if (FSWO_LOG_WARNINGS == _warningOp)
  {
    ignwarn << "Failed to open file [" << absExistingFilename << "]: "
            << std::strerror(errno) << "\n";
  }
  in.close();

  return result;
#endif
}

/////////////////////////////////////////////////
bool common::copyDirectory(const std::string &_existingDirname,
                                     const std::string &_newDirname,
                                     const FilesystemWarningOp _warningOp)
{
  // Check whether source directory exists
  if (!exists(_existingDirname) || !isDirectory(_existingDirname))
  {
    if (FSWO_LOG_WARNINGS == _warningOp)
    {
      ignwarn << "Source directory [" << _existingDirname
              << "] does not exist or is not a directory" << std::endl;
    }
    return false;
  }

  if (exists(_newDirname))
  {
    if (!removeAll(_newDirname, _warningOp))
    {
      if (FSWO_LOG_WARNINGS == _warningOp)
      {
        ignwarn << "Unable to remove existing destination directory ["
                << _newDirname << "]\n";
      }
      return false;
    }
  }
  // Create the destination directory
  if (!createDirectories(_newDirname))
  {
    if (FSWO_LOG_WARNINGS == _warningOp)
    {
      ignwarn << "Unable to create the destination directory ["
              << _newDirname << "], please check the permission\n";
    }
    return false;
  }

  // Start copy from source to destination directory
  for (DirIter file(_existingDirname); file != DirIter(); ++file)
  {
    std::string current(*file);
    if (isDirectory(current))
    {
      // Copy recursively
      if (!copyDirectory(current, joinPaths(_newDirname, basename(current)),
         _warningOp))
      {
        if (FSWO_LOG_WARNINGS == _warningOp)
        {
          ignwarn << "Unable to copy directory to ["
                  << joinPaths(_newDirname, basename(current)) << "]\n";
        }
        return false;
      }
    }
    else
    {
      if (!copyFile(current, joinPaths(_newDirname, basename(current)),
        _warningOp))
      {
        if (FSWO_LOG_WARNINGS == _warningOp)
        {
          ignwarn << "Unable to copy file to ["
                  << joinPaths(_newDirname, basename(current)) << "]\n";
        }
        return false;
      }
    }
  }
  return true;
}

/////////////////////////////////////////////////
bool common::createDirectories(const std::string &_path)
{
  std::ostringstream ss;
  auto ret = createDirectories(_path, ss);
  if (!ret)
    ignerr << ss.str();
  return ret;
}

/////////////////////////////////////////////////
bool common::createDirectories(
  const std::string &_path, std::ostream &_errorOut)
{
  size_t index = 0;
  while (index < _path.size())
  {
    size_t end = _path.find(separator(""), index+1);
    std::string dir = _path.substr(0, end);
    if (!exists(dir))
    {
#ifdef _WIN32
      dir = checkWindowsPath(dir);
      if (_mkdir(dir.c_str()) != 0)
      {
#else
      // cppcheck-suppress ConfigurationNotChecked
      if (mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0)
      {
#endif
        _errorOut << "Failed to create directory [" + dir + "]: "
                  << std::strerror(errno) << std::endl;
        return false;
      }
    }
    index = end;
  }

  return true;
}

//////////////////////////////////////////////////
std::string common::uniqueFilePath(const std::string &_pathAndName,
  const std::string &_extension)
{
  std::string result = _pathAndName + "." + _extension;
  int count = 1;

  // Check if file exists and change name accordingly
  while (exists(result.c_str()))
  {
    result = _pathAndName + "(" + std::to_string(count++) + ").";
    result += _extension;
  }

  return result;
}

//////////////////////////////////////////////////
std::string common::uniqueDirectoryPath(const std::string &_dir)
{
  std::string result = _dir;
  int count = 1;

  // Check if file exists and change name accordingly
  while (exists(result.c_str()))
  {
    result = _dir + "(" + std::to_string(count++) + ")";
  }

  return result;
}
