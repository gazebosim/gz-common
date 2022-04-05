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
#ifndef IGNITION_COMMON_TESTING_BAZELTESTPATHS_HH_
#define IGNITION_COMMON_TESTING_BAZELTESTPATHS_HH_

#include <string>

#include "ignition/common/testing/TestPaths.hh"
#include "ignition/common/testing/Export.hh"

namespace ignition::common::testing
{

/// \brief Implementation of TestPaths interface for Bazel
///
/// It is not intended that users will directly construct this, but rather
/// utilize the TestPathFactory.
///
/// The main mechanism for detecting a bazel build is via the presence of the
/// TEST_SRCDIR and TEST_UNDECLARED_OUTPUTS_DIR environment variables.
/// Additionally, tests built via ign-bazel should set IGN_BAZEL_DIR.
///
/// For source files to be available for bazel builds, they need to be set in
/// the "data" section of the relevant cc_library or cc_test call.
class BazelTestPaths: public TestPaths
{
  /// \brief Constructor from TestPaths
  public: IGNITION_COMMON_TESTING_VISIBLE using TestPaths::TestPaths;

  /// \brief Destructor
  public: IGNITION_COMMON_TESTING_VISIBLE ~BazelTestPaths() override;

  /// Documentation inherited
  public: bool IGNITION_COMMON_TESTING_VISIBLE 
          ProjectSourcePath(std::string &_sourceDir) override;

  /// Documentation inherited
  public: bool IGNITION_COMMON_TESTING_VISIBLE 
          TestTmpPath(std::string &_tmpDir) override;
};

}  // namespace ignition::common::testing

#endif  // IGNITION_COMMON_TESTING_AUTOLOGFIXTURE_HH_
