/*
* Copyright (C) 2022 Open Source Robotics Foundation
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

#include "ignition/common/testing/CMakeTestPaths.hh"

namespace ignition::common::testing
{

//////////////////////////////////////////////////
CMakeTestPaths::~CMakeTestPaths() = default;

//////////////////////////////////////////////////
bool CMakeTestPaths::ProjectSourcePath(std::string &_sourceDir) {

  if (!projectSourcePath.empty())
  {
    _sourceDir = projectSourcePath;
    return true;
  }

  return false;
}

//////////////////////////////////////////////////
bool CMakeTestPaths::TestTmpPath(std::string &_tmpDir) {
  _tmpDir = ignition::common::tempDirectoryPath();
  return true;
}

}  // namespace ignition::common::testing
