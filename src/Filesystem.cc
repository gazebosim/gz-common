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
/// \brief This file contains Apache 2.0 licensed code for filesystem functions

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

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
#include <ignition/common/ffmpeg_inc.hh>
#else
#include <io.h>
#include "ignition/common/win_dirent.h"
#endif

#include <ignition/common/Filesystem.hh>

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
bool ignition::common::isDirectory(const std::string &_path)
{
  struct stat info;

  if (stat(_path.c_str(), &info) != 0)
    return false;
  else if (info.st_mode & S_IFDIR)
    return true;
  else
    return false;
}

/////////////////////////////////////////////////
bool ignition::common::isFile(const std::string &_path)
{
  std::ifstream f(_path);
  return (!isDirectory(_path)) && f.good();
}

/////////////////////////////////////////////////
bool ignition::common::removeDirectory(const std::string &_path)
{
  if (ignition::common::isDirectory(_path))
  {
#ifdef _WIN32
    return RemoveDirectory(_path.c_str());
#else
    return rmdir(_path.c_str()) == 0;
#endif
  }

  return false;
}

/////////////////////////////////////////////////
bool ignition::common::removeDirectoryOrFile(const std::string &_path)
{
  if (ignition::common::isDirectory(_path))
  {
    return ignition::common::removeDirectory(_path);
  }
  else if (ignition::common::isFile(_path))
  {
    return std::remove(_path.c_str()) == 0;
  }
  return false;
}

/////////////////////////////////////////////////
bool ignition::common::removeAll(const std::string &_path)
{
  if (ignition::common::isDirectory(_path))
  {
    DIR *dir = opendir(_path.c_str());
    if (dir)
    {
      struct dirent *p;
      while (p=readdir(dir))
      {
        // Skip special files.
        if (!std::strcmp(p->d_name, ".") || !std::strcmp(p->d_name, ".."))
          continue;

        ignition::common::removeAll(_path + "/" + p->d_name);
      }
    }
    closedir(dir);
  }

  return ignition::common::removeDirectoryOrFile(_path);
}

/////////////////////////////////////////////////
bool ignition::common::exists(const std::string &_path)
{
  return ignition::common::isFile(_path) ||
         ignition::common::isDirectory(_path);
}

/////////////////////////////////////////////////
bool ignition::common::moveFile(const std::string &_existingFilename,
                                const std::string &_newFilename)
{
  return copyFile(_existingFilename, _newFilename) &&
         std::remove(_existingFilename.c_str()) == 0;
}

/////////////////////////////////////////////////
std::string ignition::common::absPath(const std::string &_path)
{
  std::string result;

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
  // todo #ifdef _WIN32 use PathCchCombine()
  std::string result = _path1 + "/" + _path2;
  ignition::common::replaceAll(result, result, "//", "/");
  return result;
}

/////////////////////////////////////////////////
bool ignition::common::copyFile(const std::string &_existingFilename,
                                const std::string &_newFilename)
{
  std::string absExistingFilename =
    ignition::common::absPath(_existingFilename);
  std::string absNewFilename = ignition::common::absPath(_newFilename);

  if (absExistingFilename == absNewFilename)
    return false;

#ifdef _WIN32
  return CopyFile(absExistingFilename.c_str(), absNewFilename.c_str(), false);
#elif defined(__APPLE__)
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
    out.close();
  }
  in.close();

  return result;
#else
  int readFd = 0;
  int writeFd = 0;
  struct stat statBuf;
  off_t offset = 0;

  // Open the input file.
  readFd = open(absExistingFilename.c_str(), O_RDONLY);
  if (readFd < 0)
    return false;

  // Stat the input file to obtain its size.
  fstat(readFd, &statBuf);

  // Open the output file for writing, with the same permissions as the
  // source file.
  writeFd = open(absNewFilename.c_str(), O_WRONLY | O_CREAT, statBuf.st_mode);

  while (offset >= 0 && offset < statBuf.st_size)
  {
    // Send the bytes from one file to the other.
    ssize_t written = sendfile(writeFd, readFd, &offset, statBuf.st_size);
    if (written < 0)
      break;
  }

  close(readFd);
  close(writeFd);

  return offset == statBuf.st_size;
#endif
}

/////////////////////////////////////////////////
std::string ignition::common::cwd()
{
  char buf[IGN_PATH_MAX + 1] = {'\0'};
#ifdef _WIN32
  return _getcwd(buf, sizeof(buf)) == nullptr ? "" : buf;
#else
  return getcwd(buf, sizeof(buf)) == nullptr ? "" : buf;
#endif
}

/////////////////////////////////////////////////
bool ignition::common::createDirectories(const std::string &_path)
{
  size_t index = 0;
  while (index < _path.size())
  {
    size_t end = _path.find('/', index+1);
    std::string dir = _path.substr(0, end);
    if (!exists(dir))
    {
#ifdef _WIN32
      _mkdir(dir.c_str());
#else
      mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
    }
    index = end;
  }

  return true;
}

