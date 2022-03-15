/*
 * Copyright (C) 2022 Open Source Robotics Foundation
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

#include <ignition/common/testing/TestPaths.hh>
#include <ignition/common/testing/Utils.hh>

using namespace ignition::common;
using namespace ignition::common::testing;

/////////////////////////////////////////////////
TEST(Utils, GetRandomNumber)
{
  // Force to 1
  auto val1 = getRandomNumber(1, 1);
  EXPECT_EQ("1", val1);

  // Force to 100
  auto val100 = getRandomNumber(100, 100);
  EXPECT_EQ("100", val100);


  for (size_t ii = 0; ii < 1000; ++ii)
  {
    auto val_str = getRandomNumber(0, 100);
    EXPECT_FALSE(val_str.empty());
    auto vv = std::stoi(val_str);
    EXPECT_GE(vv, 0);
    EXPECT_LE(vv, 100);
  }
}

/////////////////////////////////////////////////
TEST(Utils, CreateNewEmptyFile)
{
  auto tmpDir = MakeTestTempDirectory();
  auto path = joinPaths(tmpDir->Path(), "foobar.txt");

  ASSERT_TRUE(tmpDir->Valid());
  ASSERT_FALSE(exists(path));

  EXPECT_TRUE(createNewEmptyFile(path));
  EXPECT_TRUE(exists(path)) << path;
  EXPECT_TRUE(removeFile(path));
}

