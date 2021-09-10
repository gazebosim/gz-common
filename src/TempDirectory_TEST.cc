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

#include <ignition/utilities/ExtraTestMacros.hh>
#include <ignition/common/TempDirectory.hh>

#include <filesystem>


/////////////////////////////////////////////////
TEST(TempDirectory, tempDirectoryPath)
{
  // OS TMPDIR should never be empty
  ASSERT_FALSE(ignition::common::tempDirectoryPath().empty());
}

/////////////////////////////////////////////////
TEST(TempDirectory, createTempDirectory)
{
  // OS TMPDIR should never be empty
  auto tmpDir = ignition::common::tempDirectoryPath();
  ASSERT_FALSE(tmpDir.empty());

  // Nominal case
  // eg /tmp/fooXXXXXX
  auto tmp = ignition::common::createTempDirectory("foo", tmpDir);
  ASSERT_FALSE(tmp.empty());
  EXPECT_NE(std::string::npos, tmp.find("foo"));
  EXPECT_NE(std::string::npos, tmp.find(tmpDir));

  // Should also work for subdirectories
  // eg /tmp/bar/fooXXXXXX
  tmpDir = ignition::common::joinPaths(tmpDir, "bar");
  auto tmp2 = ignition::common::createTempDirectory("bar", tmpDir);
  ASSERT_FALSE(tmp2.empty());
}

/////////////////////////////////////////////////
TEST(TempDirectory, createTempDirectoryEmptyBase)
{
  // OS TMPDIR should never be empty
  auto tmpDir = ignition::common::tempDirectoryPath();
  ASSERT_FALSE(tmpDir.empty());

  // Create with an empty basename (eg /tmp/XXXXXX)
  auto tmp = ignition::common::createTempDirectory("", tmpDir);
  ASSERT_FALSE(tmp.empty());
  EXPECT_EQ(std::string::npos, tmp.find("foo"));
  EXPECT_NE(std::string::npos, tmp.find(tmpDir));
}

/////////////////////////////////////////////////
TEST(TempDirectory, TempDirectory)
{
  std::string path;
  {
    ignition::common::TempDirectory tmp("temp_dir", "ignition", true);
    EXPECT_TRUE(tmp.Valid());
    EXPECT_TRUE(tmp.DoCleanup());
    path = tmp.Path();
    EXPECT_FALSE(path.empty());
    EXPECT_TRUE(ignition::common::exists(path));
  }
  EXPECT_FALSE(ignition::common::exists(path));
}

/////////////////////////////////////////////////
TEST(TempDirectory, TempDirectoryNoClean)
{
  std::string path;
  {
    ignition::common::TempDirectory tmp("temp_dir", "ignition", false);
    EXPECT_TRUE(tmp.Valid());
    EXPECT_FALSE(tmp.DoCleanup());
    path = tmp.Path();
    EXPECT_FALSE(path.empty());
    EXPECT_TRUE(ignition::common::exists(path));
  }
  EXPECT_TRUE(ignition::common::exists(path));
  EXPECT_TRUE(ignition::common::removeDirectory(path));
}

/////////////////////////////////////////////////
TEST(TempDirectory, TempDirectoryNoCleanLater)
{
  std::string path;
  {
    ignition::common::TempDirectory tmp("temp_dir", "ignition", true);
    EXPECT_TRUE(tmp.Valid());
    EXPECT_TRUE(tmp.DoCleanup());
    path = tmp.Path();
    EXPECT_FALSE(path.empty());
    EXPECT_TRUE(ignition::common::exists(path));
    tmp.DoCleanup(false);
    EXPECT_FALSE(tmp.DoCleanup());
  }
  EXPECT_TRUE(ignition::common::exists(path));
  EXPECT_TRUE(ignition::common::removeDirectory(path));
}

