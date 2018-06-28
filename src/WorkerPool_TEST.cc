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

#include <atomic>
#include <gtest/gtest.h>

#include "ignition/common/Console.hh"
#include "ignition/common/WorkerPool.hh"

namespace igncmn = ignition::common;
using namespace igncmn;


//////////////////////////////////////////////////
TEST(WorkerPool, OneWorkNoCallback)
{
  WorkerPool pool;
  int workSentinel = 0;

  pool.AddWork([&workSentinel] ()
      {
        workSentinel = 5;
      });
  EXPECT_TRUE(pool.WaitForResults());
  EXPECT_EQ(5, workSentinel);
}

//////////////////////////////////////////////////
TEST(WorkerPool, OneWorkWithCallback)
{
  WorkerPool pool;
  int workSentinel = 0;
  int cbSentinel = 0;

  pool.AddWork([&workSentinel] ()
      {
        workSentinel = 5;
      },
    [&cbSentinel] ()
      {
        cbSentinel = 10;
      });
  EXPECT_TRUE(pool.WaitForResults());
  EXPECT_EQ(5, workSentinel);
  EXPECT_EQ(10, cbSentinel);
}

//////////////////////////////////////////////////
TEST(WorkerPool, LotsOfWork)
{
  WorkerPool pool;
  std::atomic<int> workSentinel(0);
  std::atomic<int> cbSentinel(0);

  for (int i = 0; i < 1000; i++)
  {
    pool.AddWork([&workSentinel] ()
        {
          workSentinel += 1;
        },
      [&cbSentinel] ()
        {
          cbSentinel += 2;
        });
  }
  EXPECT_TRUE(pool.WaitForResults());
  EXPECT_EQ(1000, workSentinel);
  EXPECT_EQ(2000, cbSentinel);
}

//////////////////////////////////////////////////
TEST(WorkerPool, WaitWithTimeout)
{
  WorkerPool pool;
  int workSentinel = 0;
  pool.AddWork([&workSentinel] ()
      {
        workSentinel = 5;
      });
  Time time(5);
  EXPECT_TRUE(pool.WaitForResults(time));
  EXPECT_EQ(5, workSentinel);
}

//////////////////////////////////////////////////
TEST(WorkerPool, WaitWithTimeoutThatTimesOut)
{
  WorkerPool pool;
  pool.AddWork([] ()
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
      });
  Time time(0.0001);
  EXPECT_FALSE(pool.WaitForResults(time));
}

//////////////////////////////////////////////////
TEST(WorkerPool, ThingsRunInParallel)
{
  const unsigned int hc = std::thread::hardware_concurrency();
  if (2 > hc)
  {
    igndbg << "Skipping the ThingsRunInParallel test because hardware "
           << "concurrency (" << hc << ") is too low (min: 2), making the test "
           << "less likely to succeed.\n";
    return;
  }

  WorkerPool pool;
  std::atomic<int> sentinel(0);
  pool.AddWork([&sentinel] ()
      {
        ++sentinel;
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
      });
  pool.AddWork([&sentinel] ()
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        ++sentinel;
      });
  Time time(0.009);
  bool result = pool.WaitForResults(time);
#ifdef __linux__
  // the timing test is flaky on windows and mac
  EXPECT_TRUE(result);
#endif
  if (!result)
  {
    igndbg << "WaitForResults failed" << std::endl;
  }
  EXPECT_EQ(2, sentinel);
}

//////////////////////////////////////////////////
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
