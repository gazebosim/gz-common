/*
 * Copyright (C) 2017 Open Source Robotics Foundation
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

#include <algorithm>
#include "gz/common/PluginLoader.hh"
#include "gz/common/SystemPaths.hh"

#include "gz/common/config.hh"
#include "test_config.h"

#ifndef IGN_COMMON_LIB_PATH
#define IGN_COMMON_LIB_PATH "./ign_common"
#endif

/////////////////////////////////////////////////
TEST(PluginLoader, InitialNoInterfacesImplemented)
{
  gz::common::PluginLoader pm;
  EXPECT_EQ(0u, pm.InterfacesImplemented().size());
}

/////////////////////////////////////////////////
TEST(PluginLoader, LoadNonexistantLibrary)
{
  gz::common::PluginLoader pm;
  EXPECT_TRUE(pm.LoadLibrary("/path/to/libDoesNotExist.so").empty());
}

/////////////////////////////////////////////////
TEST(PluginLoader, LoadNonLibrary)
{
  std::string projectPath(IGN_COMMON_LIB_PATH);
  gz::common::PluginLoader pm;
  EXPECT_TRUE(pm.LoadLibrary(projectPath + "/test_config.h").empty());
}

/////////////////////////////////////////////////
TEST(PluginLoader, LoadNonPluginLibrary)
{
  std::string libraryName("ignition-common");
  libraryName += std::to_string(IGNITION_COMMON_MAJOR_VERSION);

  gz::common::SystemPaths sp;
  sp.AddPluginPaths(IGN_COMMON_LIB_PATH);
  std::string path = sp.FindSharedLibrary(libraryName);
  ASSERT_FALSE(path.empty());

  gz::common::PluginLoader pm;
  EXPECT_TRUE(pm.LoadLibrary(path).empty());
}

/////////////////////////////////////////////////
TEST(PluginLoader, InstantiateUnloadedPlugin)
{
  gz::common::PluginLoader pm;
  gz::common::PluginPtr plugin =
      pm.Instantiate("plugin::that::is::not::loaded");
  EXPECT_FALSE(plugin);
}

/////////////////////////////////////////////////
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
