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

#include "ignition/common/Exception.hh"

/////////////////////////////////////////////////
TEST(Exception, DefaultConstructor)
{
  ignition::common::Exception exception;
  EXPECT_TRUE(exception.ErrorFile().empty());
  EXPECT_TRUE(exception.ErrorStr().empty());
}

/////////////////////////////////////////////////
TEST(Exception, InternalError)
{
  ignition::common::InternalError exception;
  EXPECT_TRUE(exception.ErrorFile().empty());
  EXPECT_TRUE(exception.ErrorStr().empty());

  ignition::common::InternalError exception2(__FILE__, __LINE__, "my_msg");
  EXPECT_FALSE(exception2.ErrorFile().empty());
  EXPECT_EQ(exception2.ErrorStr(), "my_msg");
}

/////////////////////////////////////////////////
TEST(Exception, AssertionInternalError)
{
  ignition::common::AssertionInternalError exception(__FILE__, __LINE__,
      "expr", "function", "msg");
  EXPECT_FALSE(exception.ErrorFile().empty());
  EXPECT_FALSE(exception.ErrorStr().empty());
  std::string result =
    "IGNITION ASSERTION                   \n"
    "msg\n"
    "In function       : function\n"
    "Assert expression : expr\n";
  EXPECT_EQ(exception.ErrorStr(), result);
}


/////////////////////////////////////////////////
TEST(Exception, Exception)
{
  std::string str = "test";

  try
  {
    ignthrow(str);
  }
  catch(ignition::common::Exception &_e)
  {
    _e.Print();

    EXPECT_EQ(_e.ErrorFile(), __FILE__);
    EXPECT_TRUE(_e.ErrorStr().find(str) != std::string::npos);
    return;
  }

  // should never get here
  FAIL();
}

/////////////////////////////////////////////////
TEST(Exception, InternalError_DefaultConstructor_Throw)
{
  ASSERT_THROW(ignthrow("exception"), ignition::common::Exception);
}

/////////////////////////////////////////////////
TEST(Exception, InternalError_FileLineMsgConstructor_Throw)
{
  ASSERT_THROW(ignthrow("exception"), ignition::common::Exception);
}

/////////////////////////////////////////////////
TEST(Exception, Assert)
{
  ASSERT_NO_THROW(ASSERT_DEATH(
        ign_assert(true == false, "Assert thrown"), "Assert thrown"));
}

/////////////////////////////////////////////////
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
