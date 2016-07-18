/*
 * Copyright (C) 2014 Open Source Robotics Foundation
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
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>

#include "ignition/common/SystemPaths.hh"
#include "ignition/common/Console.hh"

using namespace ignition;
using namespace common;

const std::string IGN_PLUGIN_PATH="/usr/lib/";

// Private data class
class ignition::common::SystemPathsPrivate
{
  /// \brief re-read SystemPaths#pluginPaths from environment variable
  public: void UpdatePluginPaths();

  /// \brief if true, call UpdatePluginPaths() within PluginPaths()
  public: bool pluginPathsFromEnv;

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
    mkdir(fullPath.c_str(), S_IRWXU | S_IRGRP | S_IROTH);
  }
  else
    closedir(dir);

  this->dataPtr->logPath = fullPath;

  this->dataPtr->UpdatePluginPaths();

  this->dataPtr->pluginPathsFromEnv = true;
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
const std::list<std::string> &SystemPaths::PluginPaths()
{
  if (this->dataPtr->pluginPathsFromEnv)
    this->dataPtr->UpdatePluginPaths();
  return this->dataPtr->pluginPaths;
}

//////////////////////////////////////////////////
void SystemPathsPrivate::UpdatePluginPaths()
{
  std::string delim(":");
  std::string path;

  char *pathCStr = getenv("IGN_PLUGIN_PATH");
  if (!pathCStr || *pathCStr == '\0')
  {
    // No env var; take the compile-time default.
    path = IGN_PLUGIN_PATH;
  }
  else
    path = pathCStr;

  size_t pos1 = 0;
  size_t pos2 = path.find(delim);
  while (pos2 != std::string::npos)
  {
    insertUnique(path.substr(pos1, pos2-pos1), this->pluginPaths);
    pos1 = pos2+1;
    pos2 = path.find(delim, pos2+1);
  }
  insertUnique(path.substr(pos1, path.size()-pos1), this->pluginPaths);
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

/////////////////////////////////////////////////
void SystemPaths::ClearPluginPaths()
{
  this->dataPtr->pluginPaths.clear();
}

/////////////////////////////////////////////////
void SystemPaths::AddPluginPaths(const std::string &_path)
{
  std::string delim(":");
  size_t pos1 = 0;
  size_t pos2 = _path.find(delim);
  while (pos2 != std::string::npos)
  {
    insertUnique(_path.substr(pos1, pos2-pos1), this->dataPtr->pluginPaths);
    pos1 = pos2+1;
    pos2 = _path.find(delim, pos2+1);
  }
  insertUnique(_path.substr(pos1, _path.size()-pos1),
      this->dataPtr->pluginPaths);
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
