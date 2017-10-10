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
  ASSERT_FALSE(path.empty());

  ignition::common::PluginLoader pl;

  // Make sure the expected plugins were loaded.
  std::unordered_set<std::string> pluginNames = pl.LoadLibrary(path);
  EXPECT_EQ(1u, pluginNames.count("::test::util::DummySinglePlugin"));
  EXPECT_EQ(1u, pluginNames.count("::test::util::DummyMultiPlugin"));

  std::cout << pl.PrettyStr();

  // Make sure the expected interfaces were loaded.
  EXPECT_EQ(4u, pl.InterfacesImplemented().size());
  EXPECT_EQ(1u, pl.InterfacesImplemented()
            .count("::test::util::DummyNameBase"));
  EXPECT_EQ(2u, pl.PluginsImplementing("::test::util::DummyNameBase").size());
  EXPECT_EQ(1u, pl.PluginsImplementing("::test::util::DummyDoubleBase").size());


  ignition::common::PluginPtr firstPlugin =
      pl.Instantiate("test::util::DummySinglePlugin");
  EXPECT_TRUE(firstPlugin->HasInterface("test::util::DummyNameBase"));
  EXPECT_FALSE(firstPlugin->HasInterface("test::util::DummyDoubleBase"));
  EXPECT_FALSE(firstPlugin->HasInterface("test::util::DummyIntBase"));
  EXPECT_FALSE(firstPlugin->HasInterface("test::util::DummySetterBase"));

  ignition::common::PluginPtr secondPlugin =
      pl.Instantiate("test::util::DummyMultiPlugin");
  EXPECT_TRUE(secondPlugin->HasInterface("test::util::DummyNameBase"));
  EXPECT_TRUE(secondPlugin->HasInterface("test::util::DummyDoubleBase"));
  EXPECT_TRUE(secondPlugin->HasInterface("test::util::DummyIntBase"));
  EXPECT_TRUE(secondPlugin->HasInterface("test::util::DummySetterBase"));

  // Check that the DummyNameBase interface exists and that it returns the
  // correct value.
  test::util::DummyNameBase* nameBase =
      firstPlugin->GetInterface<test::util::DummyNameBase>(
        "test::util::DummyNameBase");
  ASSERT_NE(nullptr, nameBase);
  EXPECT_EQ(std::string("DummySinglePlugin"), nameBase->MyNameIs());

  // Check that DummyDoubleBase does not exist for this plugin
  test::util::DummyDoubleBase* doubleBase =
      firstPlugin->GetInterface<test::util::DummyDoubleBase>(
        "test::util::DummyDoubleBase");
  EXPECT_EQ(nullptr, doubleBase);

  // Check that DummyDoubleBase does exist for this function and that it returns
  // the correct value.
  doubleBase = secondPlugin->GetInterface<test::util::DummyDoubleBase>(
        "test::util::DummyDoubleBase");
  ASSERT_NE(nullptr, doubleBase);
  EXPECT_NEAR(3.14159, doubleBase->MyDoubleValueIs(), 1e-8);

  // Check that the DummyNameBase interface exists for this plugin and that it
  // returns the correct value.
  nameBase = secondPlugin->GetInterface<test::util::DummyNameBase>(
        "test::util::DummyNameBase");
  ASSERT_NE(nullptr, nameBase);
  EXPECT_EQ(std::string("DummyMultiPlugin"), nameBase->MyNameIs());
}

/////////////////////////////////////////////////
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
