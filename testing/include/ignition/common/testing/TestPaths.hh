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
#ifndef IGNITION_COMMON_TESTING_TESTPATHS_HH_
#define IGNITION_COMMON_TESTING_TESTPATHS_HH_

#include <memory>
#include <string>

#include "ignition/common/Filesystem.hh"
#include "ignition/common/TempDirectory.hh"
#include "ignition/common/Util.hh"

#ifndef TESTING_PROJECT_SOURCE_DIR
#define TESTING_PROJECT_SOURCE_DIR ""
#endif

namespace ignition::common::testing
{

//////////////////////////////////////////////////
constexpr char kTestingProjectSourceDir[] = TESTING_PROJECT_SOURCE_DIR;

//////////////////////////////////////////////////
enum class BuildType
{
  kUnknown,
  kCMake,
  kBazel
};

//////////////////////////////////////////////////
class TestPaths
{
  public: explicit TestPaths(const std::string &_projectSourcePath =
              kTestingProjectSourceDir);
  public: virtual ~TestPaths() = 0;
  public: virtual bool ProjectSourcePath(std::string &_sourceDir) = 0;
  public: virtual bool TestTmpPath(std::string &_tmpDir) = 0;

  protected: std::string projectSourcePath;
};

//////////////////////////////////////////////////
std::shared_ptr<ignition::common::TempDirectory>
MakeTestTempDirectoryImpl(const std::string &_projectSourcePath,
                          const std::string &_prefix = "test",
                          const std::string &_subDir = "ignition",
                          bool _cleanup = true);


//////////////////////////////////////////////////
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
BuildType
TestBuildType(
    const std::string &_projectSourcePath = kTestingProjectSourceDir);

//////////////////////////////////////////////////
std::unique_ptr<TestPaths>
TestPathFactory(
    const std::string &_projectSourcePath = kTestingProjectSourceDir);

//////////////////////////////////////////////////
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
template <typename... Args>
std::string TestFile(Args const &... args)
{
  return SourceFile("test", args...);
}

//////////////////////////////////////////////////
template <typename... Args>
std::string TempPath(Args const &... args)
{
  auto testPaths = TestPathFactory(kTestingProjectSourceDir);
  std::string dataDir;
  testPaths->TestTmpPath(dataDir);
  return common::joinPaths(dataDir, args...);
}

}  // namespace ignition::common::testing

#endif  // IGNITION_COMMON_TESTING_TESTPATHS_HH_
