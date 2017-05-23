/*
 * Copyright (C) 2016 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>

#ifndef _WIN32
#include <dirent.h>
#else
#include "ignition/common/win_dirent.h"
#endif

#include "ignition/common/StringUtils.hh"
#include "ignition/common/SystemPaths.hh"
#include "ignition/common/Console.hh"

using namespace ignition;
using namespace common;


// Private data class
class ignition::common::SystemPathsPrivate
{
  /// \brief Name of the environment variable to check for plugin paths
  public: std::string pluginPathEnv = "IGN_PLUGIN_PATH";

  /// \brief Paths to plugins
  public: std::list<std::string> pluginPaths;

  /// \brief Suffix paths
  public: std::list<std::string> suffixPaths;

  /// \brief Log path
  public: std::string logPath;

  /// \brief Find file callback.
  public: std::function<std::string (const std::string &)> findFileCB;

  /// \brief Find file URI callback.
  public: std::function<std::string (const std::string &)> findFileURICB;

  /// \brief format the path to use "/" as a separator with "/" at the end
  public: std::string NormalizePath(const std::string &_path) const;

  /// \brief generates paths to try searching for the named library
  public: std::vector<std::string> GenerateLibraryPaths(
              const std::string &_libName) const;
};

//////////////////////////////////////////////////
/// \brief adds a path to the list if not already present
/// \param[in]_path the path
/// \param[in]_list the list
void insertUnique(const std::string &_path, std::list<std::string> &_list)
{
  if (std::find(_list.begin(), _list.end(), _path) == _list.end())
    _list.push_back(_path);
}

//////////////////////////////////////////////////
SystemPaths::SystemPaths()
: dataPtr(new SystemPathsPrivate)
{
  this->dataPtr->pluginPaths.clear();

  char *homePath = getenv("HOME");
  std::string home;
  if (!homePath)
    home = "/tmp/gazebo";
  else
    home = homePath;

  char *path = getenv("IGN_LOG_PATH");
  std::string fullPath;
  if (!path)
  {
    if (home != "/tmp/ignition")
      fullPath = home + "/.ignition";
    else
      fullPath = home;
  }
  else
    fullPath = path;

  DIR *dir = opendir(fullPath.c_str());
  if (!dir)
  {
#ifdef _WIN32
    mkdir(fullPath.c_str());
#else
    mkdir(fullPath.c_str(), S_IRWXU | S_IRGRP | S_IROTH);
#endif
  }
  else
    closedir(dir);

  this->dataPtr->logPath = fullPath;
}

/////////////////////////////////////////////////
SystemPaths::~SystemPaths()
{
}

/////////////////////////////////////////////////
std::string SystemPaths::LogPath() const
{
  return this->dataPtr->logPath;
}

/////////////////////////////////////////////////
void SystemPaths::SetPluginPathEnv(const std::string &_env)
{
  this->dataPtr->pluginPathEnv = _env;
}

/////////////////////////////////////////////////
const std::list<std::string> &SystemPaths::PluginPaths()
{
  if (this->dataPtr->pluginPathEnv.size())
  {
    char *env = getenv(this->dataPtr->pluginPathEnv.c_str());
    if (env != nullptr)
    {
      this->AddPluginPaths(std::string(env));
    }
  }
  return this->dataPtr->pluginPaths;
}

/////////////////////////////////////////////////
std::string SystemPaths::FindSharedLibrary(const std::string &_libName)
{
  // Trigger loading paths from env
  this->PluginPaths();

  std::string pathToLibrary;
  std::vector<std::string> searchNames =
    this->dataPtr->GenerateLibraryPaths(_libName);

  // TODO return list of paths that match if more than one matches?
  for (auto const &possibleName : searchNames)
  {
    if (exists(possibleName))
    {
      pathToLibrary = possibleName;
      break;
    }
  }
  return pathToLibrary;
}

/////////////////////////////////////////////////
std::string SystemPathsPrivate::NormalizePath(const std::string &_path)
  const
{
  std::string path = _path;
  // Use '/' because it works on Linux, OSX, and Windows
  std::replace(path.begin(), path.end(), '\\', '/');
  // Make last character '/'
  if (!EndsWith(path, "/"))
  {
    path += '/';
  }
  return path;
}

/////////////////////////////////////////////////
std::vector<std::string> SystemPathsPrivate::GenerateLibraryPaths(
    const std::string &_libName) const
{
  std::string lowercaseLibName = _libName;
  for (int i = 0; i < _libName.size(); ++i)
    lowercaseLibName[i] = std::tolower(_libName[i], std::locale());
  // test for possible prefixes or extensions on the library name
  bool hasLib = StartsWith(_libName, "lib");
  bool hasDotSo = EndsWith(lowercaseLibName, ".so");
  bool hasDotDll = EndsWith(lowercaseLibName, ".dll");
  bool hasDotDylib = EndsWith(lowercaseLibName, ".dylib");

  // Try removing non cross platform parts of names
  std::vector<std::string> initNames;
  initNames.push_back(_libName);
  if (hasLib && hasDotSo)
    initNames.push_back(_libName.substr(3, _libName.size() - 6));
  if (hasDotDll)
    initNames.push_back(_libName.substr(0, _libName.size() - 4));
  if (hasLib && hasDotDylib)
    initNames.push_back(_libName.substr(3, _libName.size() - 9));

  // Create possible basenames on different platforms
  std::vector<std::string> basenames;
  for (auto const &name : initNames)
  {
    basenames.push_back(name);
    basenames.push_back("lib" + name + ".so");
    basenames.push_back(name + ".so");
    basenames.push_back(name + ".dll");
    basenames.push_back("lib" + name + ".dylib");
    basenames.push_back(name + ".dylib");
    basenames.push_back("lib" + name + ".SO");
    basenames.push_back(name + ".SO");
    basenames.push_back(name + ".DLL");
    basenames.push_back("lib" + name + ".DYLIB");
    basenames.push_back(name + ".DYLIB");
  }

  std::vector<std::string> searchNames;
  // Concatenate these possible basenames with the search paths
  for (auto const &path : this->pluginPaths)
  {
    for (auto const &name : basenames)
    {
      searchNames.push_back(path + name);
    }
  }
  return searchNames;
}

//////////////////////////////////////////////////
std::string SystemPaths::FindFileURI(const std::string &_uri)
{
  int index = _uri.find("://");
  std::string prefix = _uri.substr(0, index);
  std::string suffix = _uri.substr(index + 3, _uri.size() - index - 3);
  std::string filename;

  if (prefix.empty() || prefix == "file")
  {
    // First try to find the file on the current system
    filename = this->FindFile(suffix);
  }
  else
  {
    filename = this->dataPtr->findFileURICB(_uri);
  }

  if (filename.empty())
    ignerr << "Unable to find file with URI [" << _uri << "]\n";

  return filename;
}

//////////////////////////////////////////////////
std::string SystemPaths::FindFile(const std::string &_filename,
                                  bool _searchLocalPath)
{
  std::string path;

  if (_filename.empty())
    return path;

  if (_filename.find("://") != std::string::npos)
  {
    path = this->FindFileURI(_filename);
  }
  else if (_filename[0] == '/')
  {
    path = _filename;
  }
  else
  {
    bool found = false;

    path = cwd() + "/" + _filename;

    if (_searchLocalPath && exists(path))
    {
      found = true;
    }
    else if ((_filename[0] == '/' || _filename[0] == '.' || _searchLocalPath)
             && exists(_filename))
    {
      path = _filename;
      found = true;
    }
    else
    {
      path = this->dataPtr->findFileCB(_filename);
      found = !path.empty();
    }

    if (!found)
      return std::string();
  }

  if (!exists(path))
  {
    ignerr << "File or path does not exist[" << path << "]\n";
    return std::string();
  }

  return path;
}

//////////////////////////////////////////////////
std::string SystemPaths::FindFile(const std::string &_filename,
    const std::vector<std::string> &_paths)
{
  std::string foundPath = "";
  for (auto const &path : _paths)
  {
    std::string checkPath = this->dataPtr->NormalizePath(path) + _filename;
    if (exists(checkPath))
    {
      foundPath = checkPath;
      break;
    }
  }
  return foundPath;
}

/////////////////////////////////////////////////
void SystemPaths::ClearPluginPaths()
{
  this->dataPtr->pluginPaths.clear();
}

/////////////////////////////////////////////////
void SystemPaths::AddPluginPaths(const std::string &_path)
{
  if (_path.size())
  {
#ifdef _WIN32
    char delim = ';';
#else
    char delim = ':';
#endif
    std::vector<std::string> paths = Split(_path, delim);
    for (auto const &path : paths)
    {
      std::string normalPath = this->dataPtr->NormalizePath(path);
      insertUnique(normalPath, this->dataPtr->pluginPaths);
    }
  }
}

/////////////////////////////////////////////////
void SystemPaths::AddSearchPathSuffix(const std::string &_suffix)
{
  std::string s;

  if (_suffix[0] != '/')
    s = std::string("/") + _suffix;
  else
    s = _suffix;

  if (_suffix[_suffix.size()-1] != '/')
    s += "/";

  this->dataPtr->suffixPaths.push_back(s);
}

/////////////////////////////////////////////////
void SystemPaths::SetFindFileCallback(
    std::function<std::string (const std::string &)> _cb)
{
  this->dataPtr->findFileCB = _cb;
}

/////////////////////////////////////////////////
void SystemPaths::SetFindFileURICallback(
    std::function<std::string (const std::string &)> _cb)
{
  this->dataPtr->findFileURICB = _cb;
}
