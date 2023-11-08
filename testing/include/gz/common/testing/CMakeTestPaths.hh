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
#ifndef GZ_COMMON_TESTING_CMAKETESTPATHS_HH_
#define GZ_COMMON_TESTING_CMAKETESTPATHS_HH_

#include <string>

#include "gz/common/testing/TestPaths.hh"
#include "gz/common/testing/Export.hh"

namespace gz::common::testing
{
/// \brief Implementation of TestPaths interface for CMake
///
/// It is not intended that users will directly construct this, but rather
/// utilize the TestPathFactory.

/// TODO(jrivero) Remove warning disable when bumping MSVC on CI
/// MSVC v16.11.25 complains about the TestPaths class not having a DLL although
/// relevant members of it have visiblity defined:
///   "non dll-interface class 'gz::common::testing::TestPaths' used as base for
///   dll-interface class 'gz::common::testing::CMakeTestPaths'"
///
/// Workaround on this problems triggers new warnings and suffering and
/// more recent versions of MSVC do not trigger that warning anymore.
/// Conclusion: disable the warning.
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4275)
#endif
class GZ_COMMON_TESTING_VISIBLE CMakeTestPaths: public TestPaths
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
}  // namespace gz::common::testing
# pragma warning( popd )
#endif  // GZ_COMMON_TESTING_CMAKETESTPATHS_HH_
