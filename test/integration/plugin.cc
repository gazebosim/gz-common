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

#include <algorithm>
#include <gtest/gtest.h>
#include <iostream>
#include "ignition/common/plugin/PluginLoader.hh"
#include "util/DummyPlugins.hh"


/////////////////////////////////////////////////
TEST(PluginLoader, LoadExistingLibrary)
{
  ignition::common::plugin::PluginLoader pm;
  pm.AddSearchPath("../util");
  // The search path is a little fragile
  // It requires running from directory containing this test executable
  // `make test` does this, so it's only an issue if running the test solo
  EXPECT_TRUE(pm.LoadLibrary("IGNDummyPlugins"));

  std::cout << pm.PrettyStr();

  ASSERT_EQ(1, pm.InterfacesImplemented().size());
  ASSERT_EQ("::test::util::DummyPluginBase", pm.InterfacesImplemented()[0]);
  ASSERT_EQ(1, pm.PluginsImplementing("::test::util::DummyPluginBase").size());
  std::unique_ptr<test::util::DummyPluginBase> plugin =
    pm.Instantiate<test::util::DummyPluginBase>("test::util::DummyPlugin");
  ASSERT_NE(nullptr, plugin.get());
  EXPECT_EQ(std::string("DummyPlugin"), plugin->MyNameIs());
}


/////////////////////////////////////////////////
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
