/*
 * Copyright (C) 2016 Open Source Robotics Foundation
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
#include <algorithm>
#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "ignition/common/SystemPaths.hh"

using namespace ignition;

class SystemPathsFixture : public ::testing::Test
{
  public: virtual void SetUp()
    {
      this->backupPluginPath = "IGN_PLUGIN_PATH=";

      if (getenv("IGN_PLUGIN_PATH"))
        this->backupPluginPath += getenv("IGN_PLUGIN_PATH");

      putenv(const_cast<char*>("IGN_PLUGIN_PATH="));
    }

  public: virtual void TearDown()
    {
      putenv(const_cast<char*>(this->backupPluginPath.c_str()));
    }

  public: std::string backupPluginPath;
};

TEST_F(SystemPathsFixture, SystemPaths)
{
  common::SystemPaths paths;
  putenv(const_cast<char*>("IGN_PLUGIN_PATH=/tmp/plugin:/test/plugin/now"));
  const std::list<std::string> pathList3 = paths.PluginPaths();
  EXPECT_EQ(static_cast<unsigned int>(2), pathList3.size());
  EXPECT_STREQ("/tmp/plugin/", pathList3.front().c_str());
  EXPECT_STREQ("/test/plugin/now/", pathList3.back().c_str());

  paths.ClearPluginPaths();

  EXPECT_EQ(static_cast<unsigned int>(2), paths.PluginPaths().size());
}

/////////////////////////////////////////////////
TEST_F(SystemPathsFixture, InitialNoSearchPaths)
{
  common::SystemPaths sp;
  EXPECT_EQ(0, sp.PluginPaths().size());
}

/////////////////////////////////////////////////
TEST_F(SystemPathsFixture, AddOneSearchPath)
{
  common::SystemPaths sp;
  sp.AddPluginPaths("./");
  auto paths = sp.PluginPaths();
  EXPECT_EQ(1, std::count(paths.begin(), paths.end(), "./"));
}

/////////////////////////////////////////////////
TEST_F(SystemPathsFixture, ClearSearchPaths)
{
  common::SystemPaths sp;
  sp.AddPluginPaths("./");
  EXPECT_EQ(1, sp.PluginPaths().size());
  sp.ClearPluginPaths();
  EXPECT_EQ(0, sp.PluginPaths().size());
}

/////////////////////////////////////////////////
TEST_F(SystemPathsFixture, SearchPathGetsTrailingSlash)
{
  common::SystemPaths sp;
  sp.AddPluginPaths("/usr/local/lib");
  auto paths = sp.PluginPaths();
  EXPECT_EQ(1, std::count(paths.begin(), paths.end(), "/usr/local/lib/"));
}

/////////////////////////////////////////////////
TEST_F(SystemPathsFixture, SearchPathUsesForwardSlashes)
{
#ifdef _WIN32
  std::string before = "C:\\user\\alice\\gazebolibs\\";
  std::string after = "C:/user/alice/gazebolibs/";
#else
  std::string before = "\\usr\\lib\\";
  std::string after = "/usr/lib/";
#endif
  common::SystemPaths sp;
  sp.AddPluginPaths(before);
  auto paths = sp.PluginPaths();
  EXPECT_EQ(after, *paths.begin());
}

/////////////////////////////////////////////////
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
