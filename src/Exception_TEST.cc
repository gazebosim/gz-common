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

#include "ignition/common/Util.hh"

/////////////////////////////////////////////////
TEST(Exception, Exception)
{
  try
  {
    ignthrow("test");
  }
  catch(std::runtime_error &_e)
  {
    std::cout << _e.what() << std::endl;
  }
}

/////////////////////////////////////////////////
TEST(Exception, InternalError_DefaultConstructor_Throw)
{
  ASSERT_THROW(ignthrow("exception"), ignition::common::exception);
}

/////////////////////////////////////////////////
TEST(Exception, InternalError_FileLineMsgConstructor_Throw)
{
  ASSERT_THROW(ignthrow("exception"), ignition::common::exception);
}

/////////////////////////////////////////////////
TEST(Exception, Assert)
{
  ASSERT_NO_THROW(ASSERT_DEATH(
        ignassert(true == false, "Assert thrown"), "Assert thrown"));
}

/////////////////////////////////////////////////
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
