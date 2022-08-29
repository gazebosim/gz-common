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

// Defining this macro before including ignition/common/SpecializedPluginPtr.hh
// allows us to test that the high-speed routines are being used to access the
// specialized plugin interfaces.
#define IGNITION_UNITTEST_SPECIALIZED_PLUGIN_ACCESS

#include <gtest/gtest.h>

#define SUPPRESS_IGNITION_HEADER_DEPRECATION

#include <string>
#include <vector>
#include <iostream>
#include "ignition/common/Console.hh"
#include "ignition/common/Filesystem.hh"
#include "ignition/common/SystemPaths.hh"
#include "gz/utils/SuppressWarning.hh"

#include "DummyPluginsPath.h"
#include "plugins/DummyPlugins.hh"

GZ_UTILS_WARN_IGNORE__DEPRECATED_DECLARATION
#include "ignition/common/PluginLoader.hh"
#include "ignition/common/PluginPtr.hh"
#include "ignition/common/SpecializedPluginPtr.hh"

/////////////////////////////////////////////////
TEST(PluginLoader, LoadBadPlugins)
{
  std::string dummyPath =
    gz::common::copyFromUnixPath(GZ_DUMMY_PLUGIN_PATH);

  gz::common::SystemPaths sp;
  sp.AddPluginPaths(dummyPath);

  std::vector<std::string> libraryNames = {
    "GzBadPluginAPIVersionOld",
    "GzBadPluginAPIVersionNew",
    "GzBadPluginAlign",
    "GzBadPluginSize"};
  for (auto const & libraryName : libraryNames)
  {
    std::string path = sp.FindSharedLibrary(libraryName);
    ASSERT_FALSE(path.empty());

    gz::common::PluginLoader pl;

    // Make sure the expected plugins were loaded.
    gz::common::Console::SetVerbosity(2);
    std::unordered_set<std::string> pluginNames = pl.LoadLibrary(path);
    EXPECT_TRUE(pluginNames.empty());
  }
}

/////////////////////////////////////////////////
TEST(PluginLoader, LoadExistingLibrary)
{
  std::string dummyPath =
    gz::common::copyFromUnixPath(GZ_DUMMY_PLUGIN_PATH);

  gz::common::SystemPaths sp;
  sp.AddPluginPaths(dummyPath);

  std::string path = sp.FindSharedLibrary("GzDummyPlugins");
  ASSERT_FALSE(path.empty());

  gz::common::PluginLoader pl;

  // Make sure the expected plugins were loaded.
  std::unordered_set<std::string> pluginNames = pl.LoadLibrary(path);
  EXPECT_EQ(1u, pluginNames.count("::test::util::DummySinglePlugin"));
  EXPECT_EQ(1u, pluginNames.count("::test::util::DummyMultiPlugin"));

  std::cout << pl.PrettyStr();

  // Make sure the expected interfaces were loaded.
  EXPECT_EQ(5u, pl.InterfacesImplemented().size());
  EXPECT_EQ(1u, pl.InterfacesImplemented()
            .count("::test::util::DummyNameBase"));
  EXPECT_EQ(2u, pl.PluginsImplementing("::test::util::DummyNameBase").size());
  EXPECT_EQ(1u, pl.PluginsImplementing("::test::util::DummyDoubleBase").size());


  gz::common::PluginPtr firstPlugin =
      pl.Instantiate("test::util::DummySinglePlugin");
  EXPECT_FALSE(firstPlugin.IsEmpty());
  EXPECT_TRUE(firstPlugin->HasInterface("test::util::DummyNameBase"));
  EXPECT_FALSE(firstPlugin->HasInterface("test::util::DummyDoubleBase"));
  EXPECT_FALSE(firstPlugin->HasInterface("test::util::DummyIntBase"));
  EXPECT_FALSE(firstPlugin->HasInterface("test::util::DummySetterBase"));

  gz::common::PluginPtr secondPlugin =
      pl.Instantiate("test::util::DummyMultiPlugin");
  EXPECT_FALSE(secondPlugin.IsEmpty());
  EXPECT_TRUE(secondPlugin->HasInterface("test::util::DummyNameBase"));
  EXPECT_TRUE(secondPlugin->HasInterface("test::util::DummyDoubleBase"));
  EXPECT_TRUE(secondPlugin->HasInterface("test::util::DummyIntBase"));
  EXPECT_TRUE(secondPlugin->HasInterface("test::util::DummySetterBase"));

  // Check that the DummyNameBase interface exists and that it returns the
  // correct value.
  test::util::DummyNameBase* nameBase =
      firstPlugin->QueryInterface<test::util::DummyNameBase>(
        "test::util::DummyNameBase");
  ASSERT_NE(nullptr, nameBase);
  EXPECT_EQ(std::string("DummySinglePlugin"), nameBase->MyNameIs());

  // Check that DummyDoubleBase does not exist for this plugin
  test::util::DummyDoubleBase* doubleBase =
      firstPlugin->QueryInterface<test::util::DummyDoubleBase>(
        "test::util::DummyDoubleBase");
  EXPECT_EQ(nullptr, doubleBase);

  // Check that DummyDoubleBase does exist for this function and that it returns
  // the correct value.
  doubleBase = secondPlugin->QueryInterface<test::util::DummyDoubleBase>(
        "test::util::DummyDoubleBase");
  ASSERT_NE(nullptr, doubleBase);
  EXPECT_NEAR(3.14159, doubleBase->MyDoubleValueIs(), 1e-8);

  // Check that the DummyNameBase interface exists for this plugin and that it
  // returns the correct value.
  nameBase = secondPlugin->QueryInterface<test::util::DummyNameBase>(
        "test::util::DummyNameBase");
  ASSERT_NE(nullptr, nameBase);
  EXPECT_EQ(std::string("DummyMultiPlugin"), nameBase->MyNameIs());

  test::util::DummyGetSomeObjectBase *objectBase =
    secondPlugin->QueryInterface<test::util::DummyGetSomeObjectBase>();
  ASSERT_NE(nullptr, objectBase);

  std::unique_ptr<test::util::SomeObject> object =
    objectBase->GetSomeObject();
  EXPECT_EQ(secondPlugin->QueryInterface<test::util::DummyIntBase>()
                ->MyIntegerValueIs(),
            object->someInt);
  EXPECT_NEAR(doubleBase->MyDoubleValueIs(), object->someDouble, 1e-8);
}


class SomeInterface
{
  public: IGN_COMMON_SPECIALIZE_INTERFACE(SomeInterface)
};

using SomeSpecializedPluginPtr =
    gz::common::SpecializedPluginPtr<
        SomeInterface,
        test::util::DummyIntBase,
        test::util::DummySetterBase>;

TEST(SpecializedPluginPtr, Construction)
{
  gz::common::SystemPaths sp;
  sp.AddPluginPaths(GZ_DUMMY_PLUGIN_PATH);
  std::string path = sp.FindSharedLibrary("GzDummyPlugins");
  ASSERT_FALSE(path.empty());

  gz::common::PluginLoader pl;
  pl.LoadLibrary(path);

  SomeSpecializedPluginPtr plugin(
        pl.Instantiate("::test::util::DummyMultiPlugin"));
  EXPECT_FALSE(plugin.IsEmpty());

  // Make sure the specialized interface is available, that it is accessed using
  // the specialized access, and that it returns the expected value.
  usedSpecializedInterfaceAccess = false;
  test::util::DummyIntBase *fooBase =
      plugin->QueryInterface<test::util::DummyIntBase>();
  EXPECT_TRUE(usedSpecializedInterfaceAccess);
  EXPECT_NE(nullptr, fooBase);
  EXPECT_EQ(5, fooBase->MyIntegerValueIs());

  // Make sure the specialized interface is available and that it is accessed
  // using the specialized access.
  usedSpecializedInterfaceAccess = false;
  test::util::DummySetterBase *setterBase =
      plugin->QueryInterface<test::util::DummySetterBase>();
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
      plugin->QueryInterface<test::util::DummyDoubleBase>(
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
  SomeInterface *someInterface = plugin->QueryInterface<SomeInterface>();
  EXPECT_TRUE(usedSpecializedInterfaceAccess);
  EXPECT_EQ(nullptr, someInterface);
}

template <typename PluginPtrType1, typename PluginPtrType2>
void TestSetAndMapUsage(
    const gz::common::PluginLoader &loader,
    const gz::common::PluginPtr &plugin)
{
  PluginPtrType1 plugin1 = plugin;
  PluginPtrType2 plugin2 = plugin1;

  EXPECT_TRUE(plugin1 == plugin);
  EXPECT_TRUE(plugin1 == plugin2);
  EXPECT_FALSE(plugin1 != plugin2);

  EXPECT_TRUE(plugin2 == plugin);
  EXPECT_TRUE(plugin2 == plugin1);
  EXPECT_FALSE(plugin2 != plugin1);

  std::set<gz::common::PluginPtr> orderedSet;
  EXPECT_TRUE(orderedSet.insert(plugin1).second);
  EXPECT_FALSE(orderedSet.insert(plugin1).second);
  EXPECT_FALSE(orderedSet.insert(plugin2).second);

  std::unordered_set<gz::common::PluginPtr> unorderedSet;
  EXPECT_TRUE(unorderedSet.insert(plugin1).second);
  EXPECT_FALSE(unorderedSet.insert(plugin1).second);
  EXPECT_FALSE(unorderedSet.insert(plugin2).second);

  std::map<gz::common::PluginPtr, std::string> orderedMap;
  EXPECT_TRUE(orderedMap.insert(std::make_pair(plugin1, "some string")).second);
  EXPECT_FALSE(orderedMap.insert(std::make_pair(plugin1, "a string")).second);
  EXPECT_FALSE(orderedMap.insert(std::make_pair(plugin2, "chars")).second);

  std::unordered_map<gz::common::PluginPtr, std::string> unorderedMap;
  EXPECT_TRUE(unorderedMap.insert(std::make_pair(plugin1, "strings")).second);
  EXPECT_FALSE(unorderedMap.insert(std::make_pair(plugin1, "letters")).second);
  EXPECT_FALSE(unorderedMap.insert(std::make_pair(plugin2, "")).second);


  plugin2 = loader.Instantiate("test::util::DummyMultiPlugin");
  EXPECT_TRUE(plugin1 != plugin2);
  EXPECT_FALSE(plugin1 == plugin2);
  EXPECT_TRUE(plugin2 != plugin1);
  EXPECT_FALSE(plugin2 == plugin1);

  EXPECT_TRUE(orderedSet.insert(plugin2).second);
  EXPECT_FALSE(orderedSet.insert(plugin2).second);

  EXPECT_TRUE(unorderedSet.insert(plugin2).second);
  EXPECT_FALSE(unorderedSet.insert(plugin2).second);

  EXPECT_TRUE(orderedMap.insert(std::make_pair(plugin2, "letters")).second);
  EXPECT_FALSE(orderedMap.insert(std::make_pair(plugin2, "chars")).second);

  EXPECT_TRUE(unorderedMap.insert(std::make_pair(plugin2, "abc")).second);
  EXPECT_FALSE(unorderedMap.insert(std::make_pair(plugin2, "def")).second);
}

using SingleSpecializedPluginPtr =
    gz::common::SpecializedPluginPtr<SomeInterface>;

using AnotherSpecializedPluginPtr =
    gz::common::SpecializedPluginPtr<
        SomeInterface,
        test::util::DummyIntBase>;

TEST(PluginPtr, CopyMoveSemantics)
{
  gz::common::PluginPtr plugin;
  EXPECT_TRUE(plugin.IsEmpty());

  gz::common::SystemPaths sp;
  sp.AddPluginPaths(GZ_DUMMY_PLUGIN_PATH);
  std::string path = sp.FindSharedLibrary("GzDummyPlugins");
  ASSERT_FALSE(path.empty());

  gz::common::PluginLoader pl;
  pl.LoadLibrary(path);

  plugin = pl.Instantiate("test::util::DummySinglePlugin");
  EXPECT_FALSE(plugin.IsEmpty());

  gz::common::PluginPtr otherPlugin =
      pl.Instantiate("test::util::DummySinglePlugin");
  EXPECT_FALSE(otherPlugin.IsEmpty());

  EXPECT_TRUE(plugin != otherPlugin);
  EXPECT_FALSE(plugin == otherPlugin);

  otherPlugin = plugin;
  EXPECT_TRUE(plugin == otherPlugin);
  EXPECT_FALSE(plugin != otherPlugin);

  gzdbg << "Testing sets and maps with PluginPtr and PluginPtr\n";
  TestSetAndMapUsage<
      gz::common::PluginPtr,
      gz::common::PluginPtr>(
        pl, plugin);

  gzdbg << "Testing sets and maps with PluginPtr and "
         << "SomeSpecializedPluginPtr\n";
  TestSetAndMapUsage<
      gz::common::PluginPtr,
      SomeSpecializedPluginPtr>(
        pl, plugin);

  gzdbg << "Testing sets and maps with SomeSpecializedPluginPtr and "
         << "AnotherSpecializedPluginPtr\n";
  TestSetAndMapUsage<
      SomeSpecializedPluginPtr,
      AnotherSpecializedPluginPtr>(
        pl, plugin);

  gzdbg << "Testing sets and maps with AnotherSpecializedPluginPtr and "
         << "SingleSpecializedPluginPtr\n";
  TestSetAndMapUsage<
      AnotherSpecializedPluginPtr,
      SingleSpecializedPluginPtr>(
        pl, plugin);

  gz::common::ConstPluginPtr c_plugin(plugin);
  EXPECT_FALSE(c_plugin.IsEmpty());
  EXPECT_TRUE(c_plugin == plugin);

  c_plugin = nullptr;
  EXPECT_TRUE(c_plugin.IsEmpty());

  c_plugin = otherPlugin;
  EXPECT_FALSE(c_plugin.IsEmpty());
  EXPECT_TRUE(c_plugin == otherPlugin);
}

void SetSomeValues(std::shared_ptr<test::util::DummySetterBase> setter)
{
  setter->SetIntegerValue(2468);
  setter->SetDoubleValue(6.28);
  setter->SetName("Changed using shared_ptr");
}

void CheckSomeValues(
    std::shared_ptr<test::util::DummyIntBase> getInt,
    std::shared_ptr<test::util::DummyDoubleBase> getDouble,
    std::shared_ptr<test::util::DummyNameBase> getName)
{
  EXPECT_EQ(2468, getInt->MyIntegerValueIs());
  EXPECT_NEAR(6.28, getDouble->MyDoubleValueIs(), 1e-8);
  EXPECT_EQ(std::string("Changed using shared_ptr"), getName->MyNameIs());
}

TEST(PluginPtr, QueryInterfaceSharedPtr)
{
  gz::common::SystemPaths sp;
  sp.AddPluginPaths(GZ_DUMMY_PLUGIN_PATH);
  std::string path = sp.FindSharedLibrary("GzDummyPlugins");
  ASSERT_FALSE(path.empty());

  gz::common::PluginLoader pl;
  pl.LoadLibrary(path);

  // as_shared_pointer without specialization
  {
    gz::common::PluginPtr plugin =
      pl.Instantiate("test::util::DummyMultiPlugin");

    std::shared_ptr<test::util::DummyIntBase> int_ptr =
      plugin->QueryInterfaceSharedPtr<test::util::DummyIntBase>();
    EXPECT_TRUE(int_ptr.get());
    EXPECT_EQ(5, int_ptr->MyIntegerValueIs());

    std::shared_ptr<SomeInterface> some_ptr =
      plugin->QueryInterfaceSharedPtr<SomeInterface>();
    EXPECT_FALSE(some_ptr.get());
  }

  std::shared_ptr<test::util::DummyIntBase> int_ptr =
      pl.Instantiate("test::util::DummyMultiPlugin")->
        QueryInterfaceSharedPtr<test::util::DummyIntBase>(
          "test::util::DummyIntBase");
  EXPECT_TRUE(int_ptr.get());
  EXPECT_EQ(5, int_ptr->MyIntegerValueIs());

  SomeSpecializedPluginPtr plugin =
      pl.Instantiate("test::util::DummyMultiPlugin");


  usedSpecializedInterfaceAccess = false;
  std::shared_ptr<test::util::DummySetterBase> setter =
      plugin->QueryInterfaceSharedPtr<test::util::DummySetterBase>();
  EXPECT_TRUE(usedSpecializedInterfaceAccess);
  ASSERT_TRUE(setter.get());

  usedSpecializedInterfaceAccess = false;
  std::shared_ptr<SomeInterface> someInterface =
      plugin->QueryInterfaceSharedPtr<SomeInterface>();
  EXPECT_TRUE(usedSpecializedInterfaceAccess);
  EXPECT_FALSE(someInterface.get());

  usedSpecializedInterfaceAccess = false;
  std::shared_ptr<test::util::DummyIntBase> getInt =
      plugin->QueryInterfaceSharedPtr<test::util::DummyIntBase>();
  EXPECT_TRUE(usedSpecializedInterfaceAccess);
  ASSERT_TRUE(getInt.get());

  std::shared_ptr<test::util::DummyDoubleBase> getDouble =
      plugin->QueryInterfaceSharedPtr<test::util::DummyDoubleBase>(
        "test::util::DummyDoubleBase");
  ASSERT_TRUE(getDouble.get());

  std::shared_ptr<test::util::DummyNameBase> getName =
      plugin->QueryInterfaceSharedPtr<test::util::DummyNameBase>(
        "test::util::DummyNameBase");
  ASSERT_TRUE(getName.get());

  SetSomeValues(setter);
  CheckSomeValues(getInt, getDouble, getName);
}
GZ_UTILS_WARN_RESUME__DEPRECATED_DECLARATION

#undef SUPPRESS_IGNITION_HEADER_DEPRECATION
