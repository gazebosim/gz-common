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

#include <ignition/common/config.hh>
#include <ignition/common/SystemPaths.hh>
#include <ignition/common/Util.hh>
#include <ignition/common/Uuid.hh>
#include <ignition/common/Console.hh>

#ifndef _WIN32
#include <dirent.h>
#include <limits.h>
#include <climits>
#else
#include <io.h>
#include "win_dirent.h"
#include "PrintWindowsSystemWarning.hh"
#endif

#include "ignition/common/Filesystem.hh"

#ifdef _WIN32
# define IGN_PATH_MAX _MAX_PATH
#elif defined(PATH_MAX)
# define IGN_PATH_MAX PATH_MAX
#elif defined(_XOPEN_PATH_MAX)
# define IGN_PATH_MAX _XOPEN_PATH_MAX
#else
# define IGN_PATH_MAX _POSIX_PATH_MAX
#endif

namespace igncmn = ignition::common;
using namespace ignition;
using namespace igncmn;

/////////////////////////////////////////////////
bool ignition::common::isFile(const std::string &_path)
{
  std::ifstream f(_path);
  return (!isDirectory(_path)) && f.good();
}

/////////////////////////////////////////////////
bool ignition::common::removeDirectory(const std::string &_path,
                                       const FilesystemWarningOp _warningOp)
{
  bool removed = false;
  if (ignition::common::isDirectory(_path))
  {
#ifdef _WIN32
    removed = RemoveDirectory(_path.c_str());
    if (!removed && FSWO_LOG_WARNINGS == _warningOp)
    {
      ignition::common::PrintWindowsSystemWarning(
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
bool ignition::common::removeFile(const std::string &_existingFilename,
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
bool ignition::common::removeDirectoryOrFile(
    const std::string &_path,
    const FilesystemWarningOp _warningOp)
{
  if (ignition::common::isDirectory(_path))
  {
    return ignition::common::removeDirectory(_path, _warningOp);
  }
  else if (ignition::common::isFile(_path))
  {
    return ignition::common::removeFile(_path, _warningOp);
  }
  else if (FSWO_LOG_WARNINGS == _warningOp)
  {
    ignwarn << "The path [" << _path << "] does not refer to a "
            << "directory nor to a file\n";
  }
  return false;
}

/////////////////////////////////////////////////
bool ignition::common::removeAll(const std::string &_path,
                                 const FilesystemWarningOp _warningOp)
{
  if (ignition::common::isDirectory(_path))
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

        ignition::common::removeAll(_path + "/" + p->d_name, _warningOp);
      }
    }
    closedir(dir);
  }

  return ignition::common::removeDirectoryOrFile(_path, _warningOp);
}

/////////////////////////////////////////////////
bool ignition::common::moveFile(const std::string &_existingFilename,
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
std::string ignition::common::absPath(const std::string &_path)
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
    // An absoluate path on Windows is a character followed by a colon and a
    // forward-slash.
    if (_path.compare(0, 1, "/") == 0 || _path.compare(1, 3, ":\\") == 0)
      result = _path;
    // Otherwise return the current working directory with _path appended.
    else
      result = ignition::common::cwd() + "/" + _path;
  }

  ignition::common::replaceAll(result, result, "//", "/");

  return result;
}

//////////////////////////////////////////////////
std::string ignition::common::joinPaths(const std::string &_path1,
                                        const std::string &_path2)
{
  // todo(anyone) #ifdef _WIN32 use PathCchCombine()
  return separator(_path1) + _path2;
}

/////////////////////////////////////////////////
std::string ignition::common::parentPath(const std::string &_path)
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
bool ignition::common::copyFile(const std::string &_existingFilename,
                                const std::string &_newFilename,
                                const FilesystemWarningOp _warningOp)
{
  std::string absExistingFilename =
    ignition::common::absPath(_existingFilename);
  std::string absNewFilename = ignition::common::absPath(_newFilename);

  if (absExistingFilename == absNewFilename)
    return false;

#ifdef _WIN32
  const bool copied = CopyFile(absExistingFilename.c_str(),
                               absNewFilename.c_str(), false);

  if (!copied && FSWO_LOG_WARNINGS == _warningOp)
  {
    ignition::common::PrintWindowsSystemWarning(
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
      result = ignition::common::isFile(absNewFilename);
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
bool ignition::common::copyDirectory(const std::string &_existingDirname,
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
bool ignition::common::createDirectories(const std::string &_path)
{
  size_t index = 0;
  while (index < _path.size())
  {
    size_t end = _path.find(separator(""), index+1);
    std::string dir = _path.substr(0, end);
    if (!exists(dir))
    {
#ifdef _WIN32
      _mkdir(dir.c_str());
#else
      // cppcheck-suppress ConfigurationNotChecked
      mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
    }
    index = end;
  }

  return true;
}

//////////////////////////////////////////////////
std::string ignition::common::uniqueFilePath(const std::string &_pathAndName,
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
std::string ignition::common::uniqueDirectoryPath(const std::string &_dir)
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
