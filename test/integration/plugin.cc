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

// Defining this macro before including ignition/common/SpecializedPlugin.hh
// allows us to test that the low-cost routines are being used are being used to
// access the specialized plugin interfaces.
#define IGNITION_UNITTEST_SPECIALIZED_PLUGIN_ACCESS

#include <gtest/gtest.h>
#include <iostream>
#include "ignition/common/PluginLoader.hh"
#include "ignition/common/SystemPaths.hh"
#include "ignition/common/Plugin.hh"
#include "ignition/common/SpecializedPlugin.hh"

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
  ASSERT_EQ(4u, pl.InterfacesImplemented().size());
  EXPECT_EQ(1u, pl.InterfacesImplemented()
            .count("::test::util::DummyNameBase"));
  EXPECT_EQ(2u, pl.PluginsImplementing("::test::util::DummyNameBase").size());
  EXPECT_EQ(1u, pl.PluginsImplementing("::test::util::DummyDoubleBase").size());


  std::unique_ptr<ignition::common::Plugin> firstPlugin =
      pl.Instantiate("test::util::DummySinglePlugin");
  EXPECT_NE(nullptr, firstPlugin);

  std::unique_ptr<ignition::common::Plugin> secondPlugin =
      pl.Instantiate("test::util::DummyMultiPlugin");
  EXPECT_NE(nullptr, secondPlugin);

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
  ASSERT_NE(nullptr, doubleBase);
  EXPECT_EQ(std::string("DummyMultiPlugin"), nameBase->MyNameIs());
}


class SomeInterface
{
  public: static constexpr const char* InterfaceName = "SomeInterface";
};

using SomeSpecializedPlugin =
    ignition::common::SpecializedPlugin<
        SomeInterface,
        test::util::DummyIntBase,
        test::util::DummySetterBase>;

TEST(SpecializedPlugin, Construction)
{
  std::string projectPath(PROJECT_BINARY_PATH);

  ignition::common::SystemPaths sp;
  sp.AddPluginPaths(projectPath + "/test/util");
  std::string path = sp.FindSharedLibrary("IGNDummyPlugins");
  ASSERT_FALSE(path.empty());

  ignition::common::PluginLoader pl;
  pl.LoadLibrary(path);

  std::unique_ptr<SomeSpecializedPlugin> plugin =
      pl.Instantiate<SomeSpecializedPlugin>("::test::util::DummyMultiPlugin");
  EXPECT_NE(nullptr, plugin);

  // Make sure the specialized interface is available, that it is accessed using
  // the specialized access, and that it returns the expected value.
  usedSpecializedInterfaceAccess = false;
  test::util::DummyIntBase *fooBase =
      plugin->GetInterface<test::util::DummyIntBase>();
  EXPECT_TRUE(usedSpecializedInterfaceAccess);
  EXPECT_NE(nullptr, fooBase);
  EXPECT_EQ(5, fooBase->MyIntegerValueIs());

  // Make sure the specialized interface is available and that it is accessed
  // using the specialized access.
  usedSpecializedInterfaceAccess = false;
  test::util::DummySetterBase *setterBase =
      plugin->GetInterface<test::util::DummySetterBase>();
  EXPECT_TRUE(usedSpecializedInterfaceAccess);
  EXPECT_NE(nullptr, setterBase);

  // Make sure that the interface makes the expected changes to the plugin.
  const int newIntValue = 54321;
  setterBase->SetIntegerValue(newIntValue);
  EXPECT_EQ(newIntValue, fooBase->MyIntegerValueIs());

  // Make sure the specialized interface is available, that it is accessed using
  // the specialized access, and that it returns the expected value.
  usedSpecializedInterfaceAccess = false;
  test::util::DummyDoubleBase *doubleBase =
      plugin->GetInterface<test::util::DummyDoubleBase>(
        "test::util::DummyDoubleBase");
  EXPECT_FALSE(usedSpecializedInterfaceAccess);
  EXPECT_NE(nullptr, doubleBase);
  EXPECT_NEAR(3.14159, doubleBase->MyDoubleValueIs(), 1e-8);

  // Make sure that the interfaces work together as expected.
  const double newDubValue = 2.718281828459045;
  setterBase->SetDoubleValue(newDubValue);
  EXPECT_NEAR(newDubValue, doubleBase->MyDoubleValueIs(), 1e-8);

  // Make sure that the unexpected interface is not available, even though the
  // plugin was specialized for it. Also make sure that the specialized access
  // is being used.
  usedSpecializedInterfaceAccess = false;
  SomeInterface *someInterface = plugin->GetInterface<SomeInterface>();
  EXPECT_TRUE(usedSpecializedInterfaceAccess);
  EXPECT_EQ(nullptr, someInterface);
}

/////////////////////////////////////////////////
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
