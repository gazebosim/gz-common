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
#ifndef IGNITION_COMMON_TESTING_CMAKETESTPATHS_HH_
#define IGNITION_COMMON_TESTING_CMAKETESTPATHS_HH_

#include <string>

#include "ignition/common/testing/TestPaths.hh"

namespace ignition::common::testing
{
/// \brief Implementation of TestPaths interface for CMake
///
/// It is not intended that users will directly construct this, but rather
/// utilize the TestPathFactory.
class CMakeTestPaths: public TestPaths
{
  /// \brief Constructor from TestPaths
  public: using TestPaths::TestPaths;

  /// \brief Destructor
  public: ~CMakeTestPaths() override;

  /// Documentation inherited
  public: bool ProjectSourcePath(std::string &_sourceDir) override;

  /// Documentation inherited
  public: bool TestTmpPath(std::string &_tmpDir) override;
};
}  // namespace ignition::common::testing
#endif  // IGNITION_COMMON_TESTING_CMAKETESTPATHS_HH_
