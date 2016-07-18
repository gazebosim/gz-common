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

#include <ignition/common/Timer.hh>
#include <ignition/common/Time.hh>

using namespace ignition;

TEST(TimeTest, Time)
{
  common::Timer timer;
  timer.Start();
  IGN_SLEEP_MS(100);
  EXPECT_TRUE(timer.Elapsed() > common::Time(0, 100000000));

  common::Time time;
  time = common::Time::SystemTime();
  EXPECT_TRUE(common::Time::SystemTime() - time < common::Time(0, 1000000));

  time = common::Time(1, 1000)
       + common::Time(1.5)
       + common::Time(0, static_cast<int32_t>(1e9));
  EXPECT_TRUE(time == common::Time(3,  static_cast<int32_t>(5e8 + 1000)));

  time.Set(1, 1000);
  time += common::Time(1.5);
  time += common::Time(0,  static_cast<int32_t>(1e9));
  EXPECT_TRUE(time == common::Time(3,  static_cast<int32_t>(5e8 + 1000)));

  time.Set(1, 1000);
  time -= common::Time(1, 1000);
  EXPECT_TRUE(time == common::Time(0, 0));

  time.Set(1, 1000);
  time *= common::Time(2, 2);
  EXPECT_EQ(time, common::Time(2, 2000));

  time.Set(2, 4000);
  time /= common::Time(2, 2);
  EXPECT_TRUE(time == common::Time(1, 200));
  EXPECT_FALSE(time != common::Time(1, 200));

  time += common::Time(0, 1);

  EXPECT_EQ(time, 1.0 + 201*1e-9);
  EXPECT_FALSE(time != 1.0 + 201*1e-9);
  EXPECT_TRUE(time < 2.0);
  EXPECT_TRUE(time > 0.1);
  EXPECT_TRUE(time >= 0.1);

  time.Set(1, 1000);
  time = common::Time(1, 1000) * common::Time(2, 2);
  EXPECT_TRUE(time == common::Time(2, 2000));

  time.Set(1, 1000);
  time = common::Time(1, 1000) / common::Time(2, 2);
  EXPECT_TRUE(time == common::Time(0, 500000050));

  double sec = 1.0 + 1e-9;
  double msec = sec * 1e3;
  double usec = sec * 1e6;
  double nsec = sec * 1e9;
  EXPECT_DOUBLE_EQ(nsec, IGN_SEC_TO_NANO * sec);
  EXPECT_DOUBLE_EQ(nsec, IGN_MS_TO_NANO * msec);
  EXPECT_DOUBLE_EQ(nsec, IGN_US_TO_NANO * usec);
}

/////////////////////////////////////////////////
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
