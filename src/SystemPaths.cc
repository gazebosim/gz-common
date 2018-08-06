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
#include <algorithm>
#include <cctype>
#include <cstring>
#include <functional>
#include <list>
#include <locale>
#include <string>
#include <vector>

#ifndef _WIN32
#include <dirent.h>
#else
#include "win_dirent.h"
#endif

#include "ignition/common/Console.hh"
#include "ignition/common/StringUtils.hh"
#include "ignition/common/SystemPaths.hh"
#include "ignition/common/Util.hh"

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
  public: std::function<std::string(const std::string &)> findFileCB;

  /// \brief Find file URI callback.
  public: std::function<std::string(const std::string &)> findFileURICB;

  /// \brief Callbacks to be called in order in case a file can't be found.
  public: std::vector <std::function <std::string(
              const std::string &)> > findFileCbs;

  /// \brief Callbacks to be called in order in case a file can't be found.
  public: std::vector <std::function <std::string(
              const ignition::common::URI &)> > findFileURICbs;

  /// \brief generates paths to try searching for the named library
  public: std::vector<std::string> GenerateLibraryPaths(
              const std::string &_libName) const;
};

//////////////////////////////////////////////////
/// \brief adds a path to the list if not already present
/// \param[in] _path the path
/// \param[in, out] _list the list
void insertUnique(const std::string &_path, std::list<std::string> &_list)
{
  if (std::find(_list.begin(), _list.end(), _path) == _list.end())
    _list.push_back(_path);
}

//////////////////////////////////////////////////
SystemPaths::SystemPaths()
: dataPtr(new SystemPathsPrivate)
{
  std::string home, path, fullPath;
  if (!env("HOME", home))
    home = "/tmp/gazebo";

  if (!env("IGN_LOG_PATH", path))
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
    std::string result;
    if (env(this->dataPtr->pluginPathEnv, result))
    {
      this->AddPluginPaths(result);
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
std::string SystemPaths::NormalizeDirectoryPath(const std::string &_path)
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
  for (size_t i = 0; i < _libName.size(); ++i)
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
std::string SystemPaths::FindFileURI(const std::string &_uri) const
{
  if (!ignition::common::URI::Valid(_uri))
  {
    ignerr << "The passed value [" << _uri << "] is not a valid URI, "
              "trying as a file" << std::endl;
    return this->FindFile(_uri);
  }

  // TODO: Special handling of absolute file:// URIs is needed until the URI
  //       class is fixed to support absolute URIs
  if (common::StartsWith(_uri, "file:///"))
  {
    const auto filename = _uri.substr(std::strlen("file://"));
    return this->FindFile(ignition::common::copyFromUnixPath(filename));
  }

  const auto uri = ignition::common::URI(_uri);
  return this->FindFileURI(uri);
}

//////////////////////////////////////////////////
std::string SystemPaths::FindFileURI(const ignition::common::URI &_uri) const
{
  std::string prefix = _uri.Scheme();
  std::string suffix = _uri.Path().Str() + _uri.Query().Str();
  std::string filename;

  if (prefix == "file")
  {
    // First try to find the file on the current system
    filename = this->FindFile(ignition::common::copyFromUnixPath(suffix));
  }
  else if (this->dataPtr->findFileURICB)
  {
    filename = this->dataPtr->findFileURICB(_uri.Str());
  }

  // If still not found, try custom callbacks
  if (filename.empty())
  {
    for (const auto &cb : this->dataPtr->findFileURICbs)
    {
      filename = cb(_uri);
      if (!filename.empty())
        break;
    }
  }

  if (filename.empty())
  {
    ignerr << "Unable to find file with URI [" << _uri.Str() << "]" <<
           std::endl;
    return std::string();
  }

  if (!exists(filename))
  {
    ignerr << "URI [" << _uri.Str() << "] resolved to path [" << filename <<
           "] but the path does not exist" << std::endl;
    return std::string();
  }

  return filename;
}

//////////////////////////////////////////////////
std::string SystemPaths::FindFile(const std::string &_filename,
                                  const bool _searchLocalPath) const
{
  std::string path;
  std::string filename = _filename;

  if (filename.empty())
    return path;

  // TODO: Special handling of absolute file:// URIs is needed until the URI
  //       class is fixed to support absolute URIs
  if (common::StartsWith(filename, "file:///"))
  {
    filename = filename.substr(std::strlen("file://"));
  }

  // Handle as URI
  if (ignition::common::URI::Valid(filename))
  {
    path = this->FindFileURI(ignition::common::URI(filename));
  }
  // Handle as local absolute path
  else if (filename[0] == '/')
  {
    path = filename;
  }
  // Try appending to local paths
  else
  {
    auto cwdPath = joinPaths(cwd(), filename);
    if (_searchLocalPath && exists(cwdPath))
    {
      path = cwdPath;
    }
    else if ((filename[0] == '/' || filename[0] == '.' || _searchLocalPath)
             && exists(filename))
    {
      path = filename;
    }
    else if (this->dataPtr->findFileCB)
    {
      path = this->dataPtr->findFileCB(filename);
    }
  }

  // If still not found, try custom callbacks
  if (path.empty())
  {
    for (const auto &cb : this->dataPtr->findFileCbs)
    {
      path = cb(filename);
      if (!path.empty())
        break;
    }
  }

  if (path.empty())
  {
    ignerr << "Could not resolve file [" << _filename << "]" << std::endl;
    return std::string();
  }

  if (!exists(path))
  {
    ignerr << "File [" << _filename << "] resolved to path [" << path <<
              "] but the path does not exist" << std::endl;
    return std::string();
  }

  return path;
}

//////////////////////////////////////////////////
std::string SystemPaths::LocateLocalFile(const std::string &_filename,
    const std::vector<std::string> &_paths)
{
  std::string foundPath = "";
  for (auto const &path : _paths)
  {
    std::string checkPath = NormalizeDirectoryPath(path) + _filename;
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
    std::vector<std::string> paths = Split(_path, Delimiter());
    for (auto const &path : paths)
    {
      std::string normalPath = NormalizeDirectoryPath(path);
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
    std::function<std::string(const std::string &)> _cb)
{
  this->dataPtr->findFileCB = _cb;
}

/////////////////////////////////////////////////
void SystemPaths::SetFindFileURICallback(
    std::function<std::string(const std::string &)> _cb)
{
  this->dataPtr->findFileURICB = _cb;
}

/////////////////////////////////////////////////
void SystemPaths::AddFindFileCallback(
    std::function<std::string(const std::string &)> _cb)
{
  this->dataPtr->findFileCbs.push_back(_cb);
}

/////////////////////////////////////////////////
void SystemPaths::AddFindFileURICallback(
    std::function<std::string(const ignition::common::URI &)> _cb)
{
  this->dataPtr->findFileURICbs.push_back(_cb);
}

/////////////////////////////////////////////////
std::list<std::string> SystemPaths::PathsFromEnv(const std::string &_env)
{
  std::list<std::string> paths;

  std::string envPathsStr;
  if (!env(_env, envPathsStr))
    return paths;

  if (envPathsStr.empty())
    return paths;

  auto ps = ignition::common::Split(envPathsStr, Delimiter());
  for (auto const &path : ps)
  {
    std::string normalPath = NormalizeDirectoryPath(path);
    insertUnique(normalPath, paths);
  }

  return paths;
}

/////////////////////////////////////////////////
char SystemPaths::Delimiter()
{
#ifdef _WIN32
  return ';';
#else
  return ':';
#endif
}
