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

#include <array>
#include <algorithm>
#include <sstream>
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <regex>

#include <ignition/common/config.hh>
#include <ignition/common/SystemPaths.hh>
#include <ignition/common/Util.hh>
#include <ignition/common/Uuid.hh>
#include <ignition/common/Console.hh>
#include "ignition/common/Filesystem.hh"

namespace fs = std::filesystem;

#ifndef _WIN32
static const char preferred_separator = fs::path::preferred_separator;
#else
static const char preferred_separator = '\\';
#endif

// Return true if success, false if error
inline bool fs_warn(const std::string &_fcn,
             const std::error_code &_ec,
             const ignition::common::FilesystemWarningOp &_warningOp =
             ignition::common::FSWO_LOG_WARNINGS)
{

  if (_ec)
  {
    if (ignition::common::FSWO_LOG_WARNINGS == _warningOp)
    {
      ignwarn << "Failed ignition::common::" << _fcn
        << " (ec: " << _ec << " " << _ec.message() << ")\n";
    }
    return false;
  }
  return true;
}

/////////////////////////////////////////////////
bool ignition::common::exists(const std::string &_path)
{
  return fs::exists(_path);
}

/////////////////////////////////////////////////
bool ignition::common::isDirectory(const std::string &_path)
{
  return fs::is_directory(_path);
}

/////////////////////////////////////////////////
bool ignition::common::isFile(const std::string &_path)
{
  return fs::is_regular_file(_path);
}

/////////////////////////////////////////////////
bool ignition::common::createDirectory(const std::string &_path)
{
  std::error_code ec;
  fs::create_directory(_path, ec);
  return fs_warn("createDirectory", ec);
}

/////////////////////////////////////////////////
bool ignition::common::createDirectories(const std::string &_path)
{
  std::error_code ec;
  // Disregard return here because it may return false if the
  // directory is not actually created (already exists)
  fs::create_directories(_path, ec);
  return fs_warn("createDirectories", ec);
}

/////////////////////////////////////////////////
std::string const ignition::common::separator(std::string const &_s)
{
  fs::path path(_s);
  return (_s / fs::path("")).string();
}

/////////////////////////////////////////////////
void ignition::common::changeFromUnixPath(std::string &_path) {

  std::replace(_path.begin(), _path.end(), '/', preferred_separator);
}

/////////////////////////////////////////////////
std::string ignition::common::copyFromUnixPath(const std::string &_path)
{
  std::string copy = _path;
  changeFromUnixPath(copy);
  return copy;
}

/////////////////////////////////////////////////
void ignition::common::changeToUnixPath(std::string &_path) {
  std::replace(_path.begin(), _path.end(), preferred_separator, '/');
}

/////////////////////////////////////////////////
std::string ignition::common::copyToUnixPath(const std::string &_path)
{
  std::string copy = _path;
  changeToUnixPath(copy);
  return copy;;
}

/////////////////////////////////////////////////
std::string ignition::common::absPath(const std::string &_path)
{
  return fs::absolute(_path).string();
}

/////////////////////////////////////////////////
std::string ignition::common::joinPaths(
    const std::string &_path1, const std::string &_path2)
{
  fs::path p1{_path1};
  fs::path p2{_path2};

  bool is_url = false;

  if (_path1.find("://") == std::string::npos)
    p1 = p1.lexically_normal();
  else
    is_url = true;

  if (_path2.find("://") == std::string::npos)
    p2 = p2.lexically_normal();
  else
    is_url = true;

  if (p2.string()[0] == preferred_separator)
  {
    p2 = fs::path{p2.string().substr(1)};
  }

  auto ret = (p1 / p2);

  if (!is_url)
    ret = ret.lexically_normal();

  return ret.string();
}

/////////////////////////////////////////////////
std::string ignition::common::cwd()
{
  std::error_code ec;
  auto curdir = fs::current_path(ec);

  if (!fs_warn("cwd", ec))
  {
    curdir = "";
  }

  return curdir.string();
}

/////////////////////////////////////////////////
bool ignition::common::chdir(const std::string &_dir)
{
  std::error_code ec;
  fs::current_path(_dir, ec);
  return fs_warn("chdir", ec);
}

/////////////////////////////////////////////////
std::string ignition::common::basename(const std::string &_path)
{
  fs::path p(_path);
  // Maintain compatibility with ign-common
  if (*_path.rbegin() == preferred_separator)
    p = fs::path(_path.substr(0, _path.size()-1));
  return p.filename().string();
}

/////////////////////////////////////////////////
std::string ignition::common::parentPath(const std::string &_path)
{
  fs::path p(_path);
  // Maintain compatibility with ign-common
  if (*_path.rbegin() == preferred_separator)
    p = fs::path(_path.substr(0, _path.size()-1));
  return p.parent_path().string();
}

/////////////////////////////////////////////////
bool ignition::common::copyFile(
    const std::string &_existingFilename,
    const std::string &_newFilename,
    const FilesystemWarningOp _warningOp)
{
  const auto copyOptions = fs::copy_options::overwrite_existing;
  std::error_code ec;
  auto ret = fs::copy_file(_existingFilename, _newFilename, copyOptions, ec);
  return ret && fs_warn("copyFile", ec, _warningOp);
}

/////////////////////////////////////////////////
bool ignition::common::copyDirectory(
    const std::string &_existingDirname,
    const std::string &_newDirname,
    const FilesystemWarningOp _warningOp)
{
  const auto copyOptions = fs::copy_options::recursive
                         | fs::copy_options::overwrite_existing;

  // std::filesystem won't create intermediate directories
  // before copying, this maintains compatibility with ignition behavior.
  if (!ignition::common::createDirectories(_newDirname))
  {
    return false;
  }

  std::error_code ec;
  fs::copy(_existingDirname, _newDirname, copyOptions, ec);
  return fs_warn("copyDirectory", ec, _warningOp);
}

/////////////////////////////////////////////////
bool ignition::common::moveFile(
    const std::string &_existingFilename,
    const std::string &_newFilename,
    const FilesystemWarningOp _warningOp)
{
  std::error_code ec;
  fs::rename(_existingFilename, _newFilename, ec);
  return fs_warn("moveFile", ec, _warningOp);
}

/////////////////////////////////////////////////
bool ignition::common::removeDirectory(
    const std::string &_path,
    const FilesystemWarningOp _warningOp)
{
  if (!isDirectory(_path))
  {
    if (FSWO_LOG_WARNINGS == _warningOp)
    {
      ignwarn << "Cannot remove, not a directory [" << _path << "]\n";
    }

    return false;
  }

  return removeDirectoryOrFile(_path, _warningOp);
}

/////////////////////////////////////////////////
bool ignition::common::removeFile(
    const std::string &_existingFilename,
    const FilesystemWarningOp _warningOp)
{
  if (!isFile(_existingFilename))
  {
    if (FSWO_LOG_WARNINGS == _warningOp)
    {
      ignwarn << "Cannot remove, not a file [" << _existingFilename << "]\n";
    }
    return false;
  }

  return removeDirectoryOrFile(_existingFilename, _warningOp);
}

/////////////////////////////////////////////////
bool ignition::common::removeDirectoryOrFile(
    const std::string &_path,
    const FilesystemWarningOp _warningOp)
{
  std::error_code ec;
  auto ret = fs::remove(_path, ec);
  fs_warn("removeDirectoryOrFile", ec, _warningOp);
  return ret;
}

/////////////////////////////////////////////////
bool ignition::common::removeAll(
    const std::string &_path,
    const FilesystemWarningOp _warningOp)
{
  std::error_code ec;
  fs::remove_all(_path, ec);
  return fs_warn("removeAll", ec, _warningOp);
}

/////////////////////////////////////////////////
std::string ignition::common::uniqueFilePath(
    const std::string &_pathAndName,
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

/////////////////////////////////////////////////
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
