/*
 * Copyright (C) 2016 Open Source Robotics Foundation
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

/////////////////////////////////////////////////
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
  EXPECT_TRUE(time == common::Time(1, 1999));
  EXPECT_FALSE(time != common::Time(1, 1999));

  time += common::Time(0, 1);

  EXPECT_EQ(time, 1.0 + 2000*1e-9);
  EXPECT_FALSE(time != 1.0 + 2000*1e-9);
  EXPECT_TRUE(time < 2.0);
  EXPECT_TRUE(time > 0.1);
  EXPECT_TRUE(time >= 0.1);

  time.Set(1, 1000);
  time = common::Time(1, 1000) * common::Time(2, 2);
  EXPECT_TRUE(time == common::Time(2, 2000));

  time.Set(1, 1000);
  time = common::Time(1, 1000) / common::Time(2, 2);
  EXPECT_TRUE(time == common::Time(0, 500000499));

  double sec = 1.0 + 1e-9;
  double msec = sec * 1e3;
  double usec = sec * 1e6;
  double nsec = sec * 1e9;
  EXPECT_DOUBLE_EQ(nsec, IGN_SEC_TO_NANO * sec);
  EXPECT_DOUBLE_EQ(nsec, IGN_MS_TO_NANO * msec);
  EXPECT_DOUBLE_EQ(nsec, IGN_US_TO_NANO * usec);
  EXPECT_DOUBLE_EQ(1.0, 1e9 * IGN_NANO_TO_SEC);
}

/////////////////////////////////////////////////
TEST(TimeTest, String)
{
  common::Time time(0);

  // Several combinations
  EXPECT_EQ(time.FormattedString(), "00 00:00:00.000");
  EXPECT_EQ(time.FormattedString(common::Time::FormatOption::HOURS),
                                 "00:00:00.000");
  EXPECT_EQ(time.FormattedString(common::Time::FormatOption::MINUTES),
                                 "00:00.000");
  EXPECT_EQ(time.FormattedString(common::Time::FormatOption::SECONDS),
                                 "00.000");
  EXPECT_EQ(time.FormattedString(common::Time::FormatOption::MILLISECONDS),
                                 "000");
  EXPECT_EQ(time.FormattedString(common::Time::FormatOption::DAYS,
                                 common::Time::FormatOption::MILLISECONDS),
                                 "00 00:00:00.000");
  EXPECT_EQ(time.FormattedString(common::Time::FormatOption::DAYS,
                                 common::Time::FormatOption::SECONDS),
                                 "00 00:00:00");
  EXPECT_EQ(time.FormattedString(common::Time::FormatOption::DAYS,
                                 common::Time::FormatOption::MINUTES),
                                 "00 00:00");
  EXPECT_EQ(time.FormattedString(common::Time::FormatOption::DAYS,
                                 common::Time::FormatOption::HOURS),
                                 "00 00");
  EXPECT_EQ(time.FormattedString(common::Time::FormatOption::DAYS,
                                 common::Time::FormatOption::DAYS),
                                 "00");

  // start > end: start pushes end
  EXPECT_EQ(time.FormattedString(common::Time::FormatOption::MILLISECONDS,
                                 common::Time::FormatOption::MINUTES),
                                 "000");

  // 1 second
  time = common::Time(0, 1000000000);
  EXPECT_EQ(time.FormattedString(), "00 00:00:01.000");
  EXPECT_EQ(time.FormattedString(common::Time::FormatOption::MILLISECONDS,
                                 common::Time::FormatOption::MILLISECONDS),
                                 "1000");

  // 30.5 seconds
  time = common::Time(30, 500000000);
  EXPECT_EQ(time.FormattedString(), "00 00:00:30.500");
  EXPECT_EQ(time.FormattedString(common::Time::FormatOption::SECONDS,
                                 common::Time::FormatOption::SECONDS),
                                 "30");
  EXPECT_EQ(time.FormattedString(common::Time::FormatOption::MILLISECONDS,
                                 common::Time::FormatOption::MILLISECONDS),
                                 "30500");

  // 1 min
  time = common::Time(60);
  EXPECT_EQ(time.FormattedString(), "00 00:01:00.000");
  EXPECT_EQ(time.FormattedString(common::Time::FormatOption::SECONDS,
                                 common::Time::FormatOption::SECONDS),
                                 "60");

  // 2.5 hours
  time = common::Time(9000);
  EXPECT_EQ(time.FormattedString(), "00 02:30:00.000");
  EXPECT_EQ(time.FormattedString(common::Time::FormatOption::HOURS,
                                 common::Time::FormatOption::MINUTES),
                                 "02:30");
  EXPECT_EQ(time.FormattedString(common::Time::FormatOption::SECONDS,
                                 common::Time::FormatOption::SECONDS),
                                 "9000");
  EXPECT_EQ(time.FormattedString(common::Time::FormatOption::MINUTES,
                                 common::Time::FormatOption::MINUTES),
                                 "150");
  EXPECT_EQ(time.FormattedString(common::Time::FormatOption::HOURS,
                                 common::Time::FormatOption::HOURS),
                                 "02");

  // 3 days
  time = common::Time(259200);
  EXPECT_EQ(time.FormattedString(), "03 00:00:00.000");
  EXPECT_EQ(time.FormattedString(common::Time::FormatOption::DAYS,
                                 common::Time::FormatOption::DAYS),
                                 "03");
  EXPECT_EQ(time.FormattedString(common::Time::FormatOption::HOURS),
                                 "72:00:00.000");
  EXPECT_EQ(time.FormattedString(common::Time::FormatOption::HOURS,
                                 common::Time::FormatOption::HOURS),
                                 "72");
  EXPECT_EQ(time.FormattedString(common::Time::FormatOption::MINUTES,
                                 common::Time::FormatOption::MINUTES),
                                 "4320");

  // Large time
  time = common::Time(1234567890, 123456789);
  EXPECT_EQ(time.FormattedString(), "14288 23:31:30.123");
}

/////////////////////////////////////////////////
TEST(TimeTest, Double)
{
  common::Time time(1, 900000000);
  EXPECT_DOUBLE_EQ(1.9, time.Double());

  time.Set(1, -900000000);
  EXPECT_DOUBLE_EQ(0.100000000, time.Double());

  time.Set(-1, 900000000);
  EXPECT_DOUBLE_EQ(-0.100000000, time.Double());

  time.Set(-1, -900000000);
  EXPECT_DOUBLE_EQ(-1.900000000, time.Double());
}

/////////////////////////////////////////////////
TEST(TimeTest, Float)
{
  common::Time time(1, 900000000);
  EXPECT_FLOAT_EQ(1.9f, time.Float());

  time.Set(1, -900000000);
  EXPECT_FLOAT_EQ(0.100000000f, time.Float());

  time.Set(-1, 900000000);
  EXPECT_FLOAT_EQ(-0.100000000f, time.Float());

  time.Set(-1, -900000000);
  EXPECT_FLOAT_EQ(-1.900000000f, time.Float());
}

/////////////////////////////////////////////////
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
