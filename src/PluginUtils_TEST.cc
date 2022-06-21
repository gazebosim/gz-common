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

#define SUPPRESS_IGNITION_HEADER_DEPRECATION

#include <string>
#include "gz/utils/SuppressWarning.hh"

#include "PluginUtils.hh"

using namespace gz;

GZ_UTILS_WARN_IGNORE__DEPRECATED_DECLARATION
/////////////////////////////////////////////////
TEST(PluginUtils, NormalizeName)
{
  EXPECT_EQ("::", common::NormalizeName(""));
  EXPECT_EQ("::", common::NormalizeName("::"));

  EXPECT_EQ("::ignition", common::NormalizeName("ignition"));
  EXPECT_EQ("::ignition", common::NormalizeName("::ignition"));

  EXPECT_EQ("::gz::math", common::NormalizeName("gz::math"));
  EXPECT_EQ("::gz::math", common::NormalizeName("::gz::math"));
}
GZ_UTILS_WARN_RESUME__DEPRECATED_DECLARATION

#undef SUPPRESS_IGNITION_HEADER_DEPRECATION
