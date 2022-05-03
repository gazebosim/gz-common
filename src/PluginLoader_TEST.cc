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
#include <fstream>

#include <gz/utils/SuppressWarning.hh>

#include "ignition/common/config.hh"
#include "ignition/common/SystemPaths.hh"
#include "ignition/common/TempDirectory.hh"

#include "ignition/common/testing/Utils.hh"

/////////////////////////////////////////////////
class TestTempDirectory : public ignition::common::TempDirectory
{
  public: TestTempDirectory():
    ignition::common::TempDirectory("plugin_loader", "ign_common", true)
  {
  }
};

IGN_UTILS_WARN_IGNORE__DEPRECATED_DECLARATION
#include "ignition/common/PluginLoader.hh"

/////////////////////////////////////////////////
TEST(PluginLoader, InitialNoInterfacesImplemented)
{
  ignition::common::PluginLoader pm;
  EXPECT_EQ(0u, pm.InterfacesImplemented().size());
}

/////////////////////////////////////////////////
TEST(PluginLoader, LoadNonexistantLibrary)
{
  ignition::common::PluginLoader pm;
  EXPECT_TRUE(pm.LoadLibrary("/path/to/libDoesNotExist.so").empty());
}

/////////////////////////////////////////////////
TEST(PluginLoader, LoadNonLibrary)
{
  TestTempDirectory tempDir;
  ignition::common::testing::createNewEmptyFile("not_a_library.txt");

  ignition::common::PluginLoader pm;
  EXPECT_TRUE(pm.LoadLibrary("not_a_library.txt").empty());
}

/////////////////////////////////////////////////
TEST(PluginLoader, LoadNonPluginLibrary)
{
  std::string libDir = "lib_dir";
  std::string libName = "foobar";

  TestTempDirectory tempDir;
  ignition::common::createDirectory(libDir);
  ignition::common::testing::createNewEmptyFile(
      ignition::common::joinPaths(libDir, "lib" + libName + ".so"));

  ignition::common::SystemPaths sp;

  // Fails without plugin dirs setup
  std::string path = sp.FindSharedLibrary("foo");
  ASSERT_TRUE(path.empty());

  sp.AddPluginPaths(
      ignition::common::joinPaths(ignition::common::cwd(), libDir));
  path = sp.FindSharedLibrary(libName);
  ASSERT_FALSE(path.empty());

  ignition::common::PluginLoader pm;
  EXPECT_TRUE(pm.LoadLibrary(path).empty());
}

/////////////////////////////////////////////////
TEST(PluginLoader, InstantiateUnloadedPlugin)
{
  ignition::common::PluginLoader pm;
  ignition::common::PluginPtr plugin =
      pm.Instantiate("plugin::that::is::not::loaded");
  EXPECT_FALSE(plugin);
}
IGN_UTILS_WARN_RESUME__DEPRECATED_DECLARATION
