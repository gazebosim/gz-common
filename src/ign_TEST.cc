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

#include <gtest/gtest.h>
#include <cstdio>
#include <cstdlib>

#include <string>
#include <ign.hh>
#include <ignition/common/Console.hh>
#include <ignition/common/Util.hh>

using namespace ignition::common;

/////////////////////////////////////////////////
TEST(CBindings, Verbosity)
{
  cmdVerbosity("4");
  EXPECT_EQ(4, Console::Verbosity());

  cmdVerbosity("3");
  EXPECT_EQ(3, Console::Verbosity());

  cmdVerbosity("2");
  EXPECT_EQ(2, Console::Verbosity());

  cmdVerbosity("1");
  EXPECT_EQ(1, Console::Verbosity());
}

/////////////////////////////////////////////////
TEST(CBindings, FindFile)
{
  const auto kEnvName = "IGN_TEST_PATH";
  const auto thisFilePath = __FILE__;
  const auto thisFile = ignition::common::basename(thisFilePath);
  const auto thisDir = parentPath(thisFilePath);
  const auto projectDir = parentPath(thisDir);

  setenv(kEnvName, thisDir);
  auto path = findFileInPathEnv(thisFile.c_str(), kEnvName);
  EXPECT_STREQ(thisFilePath, path);

  setenv(kEnvName, thisDir + SystemPaths::Delimiter() + "foo");
  path = findFileInPathEnv(thisFile.c_str(), kEnvName);
  EXPECT_STREQ(thisFilePath, path);

  setenv(kEnvName, std::string("foo") + SystemPaths::Delimiter() + thisDir);
  path = findFileInPathEnv(thisFile.c_str(), kEnvName);
  EXPECT_STREQ(thisFilePath, path);

  setenv(kEnvName, "foo");
  path = findFileInPathEnv(thisFile.c_str(), kEnvName, thisDir.c_str());
  EXPECT_STREQ(thisFilePath, path);

  setenv(kEnvName, thisDir);
  path = findFileInPathEnv(thisFile.c_str(), kEnvName, "foo");
  EXPECT_STREQ(thisFilePath, path);

  // empty search path means the file should not be found
  setenv(kEnvName, "");
  path = findFileInPathEnv(thisFile.c_str(), kEnvName);
  EXPECT_STREQ("", path);

  const auto thisCMakeLists = joinPaths(thisDir, "CMakeLists.txt");
  const auto projectCMakeLists = joinPaths(projectDir, "CMakeLists.txt");
  ASSERT_TRUE(exists(thisCMakeLists));
  ASSERT_TRUE(exists(projectCMakeLists));

  // paths from environment take precedence over default paths
  setenv(kEnvName, thisDir);
  path = findFileInPathEnv("CMakeLists.txt", kEnvName, projectDir.c_str());
  EXPECT_STREQ(thisCMakeLists.c_str(), path);

  // paths from environment are searched left to right
  setenv(kEnvName, thisDir + SystemPaths::Delimiter() + projectDir);
  path = findFileInPathEnv("CMakeLists.txt", kEnvName);
  EXPECT_STREQ(thisCMakeLists.c_str(), path);
}

/////////////////////////////////////////////////
int main(int _argc, char **_argv)
{
  ::testing::InitGoogleTest(&_argc, _argv);
  return RUN_ALL_TESTS();
}
