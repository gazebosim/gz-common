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

#include <boost/filesystem.hpp>
#include <iostream>
#include <fstream>
#include <sstream>

#include <sdf/sdf.hh>

#include "ignition/common/SystemPaths.hh"
#include "ignition/common/Exception.hh"
#include "ignition/common/Console.hh"

using namespace ignition;
using namespace common;

//////////////////////////////////////////////////
SystemPaths::SystemPaths()
{
  this->pluginPaths.clear();

  char *homePath = getenv("HOME");
  std::string home;
  if (!homePath)
    home = "/tmp/gazebo";
  else
    home = homePath;

  char *path = getenv("GAZEBO_LOG_PATH");
  std::string fullPath;
  if (!path)
  {
    if (home != "/tmp/gazebo")
      fullPath = home + "/.gazebo";
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

  this->logPath = fullPath;

  this->UpdatePluginPaths();

  this->pluginPathsFromEnv = true;
}

/////////////////////////////////////////////////
SystemPaths::~SystemPaths()
{
}

/////////////////////////////////////////////////
std::string SystemPaths::GetLogPath() const
{
  return this->logPath;
}

/////////////////////////////////////////////////
const std::list<std::string> &SystemPaths::GetPluginPaths()
{
  if (this->pluginPathsFromEnv)
    this->UpdatePluginPaths();
  return this->pluginPaths;
}

//////////////////////////////////////////////////
void SystemPaths::UpdatePluginPaths()
{
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
    filename = this->FindFileURIHelper(_uri);
  }

  if (filename.empty())
    ignerr << "Unable to find file with URI [" << _uri << "]\n";

  return filename;
}

//////////////////////////////////////////////////
std::string SystemPaths::FindFileURIHelper(const std::string & /*_uri*/)
{
  return std::string();
}

//////////////////////////////////////////////////
std::string SystemPaths::FindFileHelper(const std::string & /*_filename*/)
{
  return std::string();
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

    try
    {
      path = boost::filesystem::current_path().string() + "/" + _filename;
    }
    catch(boost::filesystem::filesystem_error &_e)
    {
      ignerr << "Filesystem error[" << _e.what() << "]\n";
      return std::string();
    }

    if (_searchLocalPath && boost::filesystem::exists(path))
    {
      found = true;
    }
    else if ((_filename[0] == '/' || _filename[0] == '.' || _searchLocalPath)
             && boost::filesystem::exists(boost::filesystem::path(_filename)))
    {
      path = _filename;
      found = true;
    }
    else
    {
      path = this->FindFileHelper(_filename);
      found = !path.empty();
    }

    if (!found)
      return std::string();
  }

  if (!boost::filesystem::exists(path))
  {
    ignerr << "File or path does not exist[" << path << "]\n";
    return std::string();
  }

  return path;
}

/////////////////////////////////////////////////
void SystemPaths::ClearPluginPaths()
{
  this->pluginPaths.clear();
}

/////////////////////////////////////////////////
void SystemPaths::AddPluginPaths(const std::string &_path)
{
  std::string delim(":");
  size_t pos1 = 0;
  size_t pos2 = _path.find(delim);
  while (pos2 != std::string::npos)
  {
    this->InsertUnique(_path.substr(pos1, pos2-pos1), this->pluginPaths);
    pos1 = pos2+1;
    pos2 = _path.find(delim, pos2+1);
  }
  this->InsertUnique(_path.substr(pos1, _path.size()-pos1), this->pluginPaths);
}

/////////////////////////////////////////////////
void SystemPaths::InsertUnique(const std::string &_path,
                               std::list<std::string> &_list)
{
  if (std::find(_list.begin(), _list.end(), _path) == _list.end())
    _list.push_back(_path);
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

  this->suffixPaths.push_back(s);
}
