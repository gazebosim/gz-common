/*
 * Copyright (C) 2021 Open Source Robotics Foundation
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

#include <cstring>

#include <ignition/common/Console.hh>
#include <ignition/common/SystemPaths.hh>

#include "ignition/common/config.hh"
#include "ign.hh"

//////////////////////////////////////////////////
extern "C" IGNITION_COMMON_VISIBLE char* ignitionCommonVersion()
{
  return strdup(IGNITION_COMMON_VERSION_FULL);
}

//////////////////////////////////////////////////
extern "C" IGNITION_COMMON_VISIBLE void cmdVerbosity(
  const char* _verbosity)
{
  ignition::common::Console::SetVerbosity(std::atoi(_verbosity));
}

//////////////////////////////////////////////////
extern "C" IGNITION_COMMON_VISIBLE char* findFileInPathEnv(
  const char* _fileName, const char* _envName, const char* _defaultPath)
{
  ignition::common::SystemPaths paths;
  paths.SetFilePathEnv(_envName);
  if (strlen(_defaultPath) > 0)
    paths.AddFilePaths(_defaultPath);

  const auto foundPath = paths.FindFile(_fileName, false);
  return strdup(ignition::common::copyFromUnixPath(foundPath).c_str());
}
