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
#include <iostream>
#include "ignition/common/PluginLoader.hh"
#include "ignition/common/SystemPaths.hh"
#include "ignition/common/Plugin.hh"

#include "test_config.h"
#include "util/DummyPlugins.hh"

/////////////////////////////////////////////////
TEST(PluginLoader, LoadExistingLibrary)
{
  std::string projectPath(PROJECT_BINARY_PATH);

  ignition::common::SystemPaths sp;
  sp.AddPluginPaths(projectPath + "/test/util");
  std::string path = sp.FindSharedLibrary("IGNDummyPlugins");
  ASSERT_LT(0, path.size());

  ignition::common::PluginLoader pm;

  std::unordered_set<std::string> pluginNames = pm.LoadLibrary(path);
  EXPECT_EQ(1u, pluginNames.count("::test::util::DummyPlugin"));
  EXPECT_EQ(1u, pluginNames.count("::test::util::DummyMultiPlugin"));

  std::cout << pm.PrettyStr();

  ASSERT_EQ(2u, pm.InterfacesImplemented().size());
  EXPECT_EQ(1u, pm.InterfacesImplemented().count("::test::util::DummyPluginBase"));
  EXPECT_EQ(2u, pm.PluginsImplementing("::test::util::DummyPluginBase").size());
  EXPECT_EQ(1u, pm.PluginsImplementing("::test::util::DummyOtherBase").size());


  std::unique_ptr<ignition::common::Plugin> firstPlugin =
      pm.Instantiate("test::util::DummyPlugin");
  std::unique_ptr<ignition::common::Plugin> secondPlugin =
      pm.Instantiate("test::util::DummyMultiPlugin");

  test::util::DummyPluginBase* dummyBase =
      firstPlugin->GetInterface<test::util::DummyPluginBase>(
        "test::util::DummyPluginBase");
  ASSERT_NE(nullptr, dummyBase);
  EXPECT_EQ(std::string("DummyPlugin"), dummyBase->MyNameIs());

  test::util::DummyOtherBase* otherBase =
      firstPlugin->GetInterface<test::util::DummyOtherBase>(
        "test::util::DummyOtherBase");
  EXPECT_EQ(nullptr, otherBase);

  otherBase = secondPlugin->GetInterface<test::util::DummyOtherBase>(
        "test::util::DummyOtherBase");
  ASSERT_NE(nullptr, otherBase);
  EXPECT_EQ(3.14159, otherBase->MyValueIs());

  dummyBase = secondPlugin->GetInterface<test::util::DummyPluginBase>(
        "test::util::DummyPluginBase");
  ASSERT_NE(nullptr, otherBase);
  EXPECT_EQ(std::string("DummyMultiPlugin"), dummyBase->MyNameIs());
}

/////////////////////////////////////////////////
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
