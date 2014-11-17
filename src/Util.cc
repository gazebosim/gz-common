/*
 * Copyright (C) 2012-2014 Open Source Robotics Foundation
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
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <ignition/common/config.hh>
#include <ignition/common/SystemPaths.hh>
#include <ignition/common/Util.hh>

/////////////////////////////////////////////////
std::string ignition::common::SystemTimeISO()
{
  char isoStr[25];

  auto epoch = ign_system_time().time_since_epoch();
  auto sec = std::chrono::duration_cast<std::chrono::seconds>(epoch).count();
  auto nano = std::chrono::duration_cast<std::chrono::nanoseconds>(
      epoch).count() - sec * IGN_SEC_TO_NANO;

  std::strftime(isoStr, sizeof(isoStr), "%FT%T", std::localtime(&sec));

  return std::string(isoStr) + "." + std::to_string(nano);
}

/////////////////////////////////////////////////
void ignition::common::addSearchPathSuffix(const std::string & /*_suffix*/)
{
  // common::SystemPaths::Instance()->AddSearchPathSuffix(_suffix);
}

/////////////////////////////////////////////////
std::string ignition::common::findFile(const std::string & /*_file*/)
{
  return "";
  // return common::SystemPaths::Instance()->FindFile(_file, true);
}

/////////////////////////////////////////////////
std::string ignition::common::findFile(const std::string & /*_file*/,
    bool /*_searchLocalPath*/)
{
  return "";
  // return common::SystemPaths::Instance()->FindFile(_file, _searchLocalPath);
}

/////////////////////////////////////////////////
std::string ignition::common::findFilePath(const std::string &_file)
{
  std::string filepath = findFile(_file);

  boost::filesystem::path path(filepath);
  if (boost::filesystem::is_directory(path))
  {
    return filepath;
  }
  else
  {
    int index = filepath.find_last_of("/");
    return filepath.substr(0, index);
  }
}
