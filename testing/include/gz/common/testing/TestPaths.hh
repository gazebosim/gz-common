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
#ifndef GZ_COMMON_TESTING_TESTPATHS_HH_
#define GZ_COMMON_TESTING_TESTPATHS_HH_

#include <memory>
#include <string>

#include "gz/common/Filesystem.hh"
#include "gz/common/TempDirectory.hh"
#include "gz/common/Util.hh"

#include "gz/common/testing/Export.hh"

#ifndef TESTING_PROJECT_SOURCE_DIR
#define TESTING_PROJECT_SOURCE_DIR ""
#endif

namespace ignition::common::testing
{

//////////////////////////////////////////////////
/// \brief Constant referring to the project source dir of the current
/// project.
///
/// For CMake builds, it is expected that this is injected via a
/// buildsystem define of TESTING_PROJECT_SOURCE_DIR.
/// This is done automatically for tests using ign-cmake's ign_build_tests
///
/// For Bazel builds, it is expected to be empty
constexpr char kTestingProjectSourceDir[] = TESTING_PROJECT_SOURCE_DIR;

//////////////////////////////////////////////////
/// \brief List of buildsystem types
enum class IGNITION_COMMON_TESTING_VISIBLE BuildType
{
  kUnknown,
  kCMake,
  kBazel
};

//////////////////////////////////////////////////
/// \brief Helper interface to generate path information to support
/// test access to source/data files
///
/// It is intended that there is an implementation of this interface for
/// each relevant buildsystem.
class TestPaths
{
  /// \brief Constructor
  /// \param[in] _projectSourcePath Path to the root of the project source
  public: IGNITION_COMMON_TESTING_VISIBLE
          explicit TestPaths(const std::string &_projectSourcePath =
              kTestingProjectSourceDir);

  /// \brief Destructor
  public: IGNITION_COMMON_TESTING_VISIBLE
          virtual ~TestPaths() = 0;

  /// brief Populate the path to the root project source directory
  /// \param[out] _sourceDir path to the root project source directory
  /// \return True if path successfully found and set, false otherwise
  public: virtual bool IGNITION_COMMON_TESTING_VISIBLE
          ProjectSourcePath(std::string &_sourceDir) = 0;

  /// \brief Populate the path to a temporary directory
  /// \param[out] _tmpDir path to the root temporary directory
  /// \return True if path successfully found and set, false otherwise
  public: virtual bool IGNITION_COMMON_TESTING_VISIBLE
          TestTmpPath(std::string &_tmpDir) = 0;

  /// \brief Path to the root of the project source
  protected: std::string projectSourcePath;
};

//////////////////////////////////////////////////
/// \brief Implementation of MakeTestTempDirectory
///
/// The TempDirectory will have the form $TMPDIR/_subdir/_prefixXXXXX/
///
/// \param[in] _projectSourcePath Root of project source or empty
/// \param[in] _prefix Prefix of the temporary directory
/// \param[in] _subDir Additional subdirectory for temporary directory
/// \param[in] _cleanup True to indicate that the filesystem should
///   be cleaned as part of the destructor
/// \return Shared pointer to TempDirectory
std::shared_ptr<ignition::common::TempDirectory>
IGNITION_COMMON_TESTING_VISIBLE
MakeTestTempDirectoryImpl(const std::string &_projectSourcePath,
                          const std::string &_prefix = "test",
                          const std::string &_subDir = "ignition",
                          bool _cleanup = true);


//////////////////////////////////////////////////
/// \brief Create a temporary directory for test output in an OS and build
///   appropriate location
///
/// The TempDirectory will have the form $TMPDIR/_subdir/_prefixXXXXX/
///
/// \param[in] _prefix Prefix of the temporary directory
/// \param[in] _subDir Additional subdirectory for temporary directory
/// \param[in] _cleanup True to indicate that the filesystem should
///   be cleaned as part of the destructor
/// \return Shared pointer to TempDirectory
inline std::shared_ptr<ignition::common::TempDirectory>
MakeTestTempDirectory(const std::string &_prefix = "test",
                      const std::string &_subDir = "ignition",
                      bool _cleanup = true)
{
  return MakeTestTempDirectoryImpl(kTestingProjectSourceDir,
                                   _prefix,
                                   _subDir,
                                   _cleanup);
}

//////////////////////////////////////////////////
/// \brief Return the current build type
///
/// \param[in] _projectSourcePath Root of project source or empty
/// \return The current build type
BuildType
IGNITION_COMMON_TESTING_VISIBLE
TestBuildType(
    const std::string &_projectSourcePath = kTestingProjectSourceDir);

//////////////////////////////////////////////////
/// \brief Get a TestPaths object for the current build type
///
/// \param[in] _projectSourcePath Root of project source or empty
/// \return TestPaths implementation for the current build type
std::unique_ptr<TestPaths>
IGNITION_COMMON_TESTING_VISIBLE
TestPathFactory(
    const std::string &_projectSourcePath = kTestingProjectSourceDir);

//////////////////////////////////////////////////
/// \brief Get the path to a file in the project source tree
///
/// Example: to get ign-common/test/data/box.dae
/// SourceFile("test", "data", "box.dae");
///
/// \param[in] args Relative path to the source file
/// \return Full path to the source file
template <typename... Args>
std::string SourceFile(Args const &... args)
{
  auto testPaths = TestPathFactory(kTestingProjectSourceDir);
  assert(nullptr != testPaths);

  std::string dataDir;
  testPaths->ProjectSourcePath(dataDir);
  return common::joinPaths(dataDir, args...);
}

//////////////////////////////////////////////////
/// \brief Get the path to a file in the project test directory tree
///
/// Example: to get ign-common/test/data/box.dae
/// TestFile("data", "box.dae");
///
/// \param[in] args Path to the file, relative to the test directory
/// \return Full path to the source file
template <typename... Args>
std::string TestFile(Args const &... args)
{
  return SourceFile("test", args...);
}

//////////////////////////////////////////////////
/// \brief Get the path to a file in a temporary directory
///
/// Example: to get ${TMP}/.ignition/foo.log
///  TempPath(".ignition", "foo.log");
///
/// \param[in] args Path to the file, relative to the temporary directory
/// \return Full path to the temporary directory
template <typename... Args>
std::string TempPath(Args const &... args)
{
  auto testPaths = TestPathFactory(kTestingProjectSourceDir);
  std::string dataDir;
  testPaths->TestTmpPath(dataDir);
  return common::joinPaths(dataDir, args...);
}
}  // namespace ignition::common::testing
#endif  // GZ_COMMON_TESTING_TESTPATHS_HH_
