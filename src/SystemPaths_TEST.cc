/*
 * Copyright (C) 2012-2014 Open Source Robotics Foundation
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
#include <string>
#include <vector>


#include "ignition/common/SystemPaths.hh"

using namespace ignition;

TEST(SystemPathsTest, SystemPaths)
{
  std::string pluginPathBackup = "IGN_PLUGIN_PATH=";

  if (getenv("IGN_PLUGIN_PATH"))
    pluginPathBackup += getenv("IGN_PLUGIN_PATH");

  putenv(const_cast<char*>("IGN_LOG_PATH="));
  common::SystemPaths *paths = new common::SystemPaths();

  paths->ClearPluginPaths();

  putenv(const_cast<char*>("IGN_PLUGIN_PATH=/tmp/plugin:/test/plugin/now"));
  const std::list<std::string> pathList3 = paths->PluginPaths();
  EXPECT_EQ(static_cast<unsigned int>(2), pathList3.size());
  EXPECT_STREQ("/tmp/plugin", pathList3.front().c_str());
  EXPECT_STREQ("/test/plugin/now", pathList3.back().c_str());

  paths->ClearPluginPaths();

  EXPECT_EQ(static_cast<unsigned int>(2), paths->PluginPaths().size());

  putenv(const_cast<char*>("IGN_PLUGIN_PATH="));
  paths->ClearPluginPaths();

  putenv(const_cast<char*>(pluginPathBackup.c_str()));

  delete paths;
}

/////////////////////////////////////////////////
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
