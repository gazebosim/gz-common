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

#include <atomic>
#include <chrono>
#include <future>
#include <thread>

#include "gz/common/Console.hh"
#include "gz/common/WorkerPool.hh"

using namespace gz;

//////////////////////////////////////////////////
TEST(WorkerPool, OneWorkNoCallback)
{
  common::WorkerPool pool;
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
  common::WorkerPool pool;
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
  common::WorkerPool pool;
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
  common::WorkerPool pool;
  int workSentinel = 0;
  pool.AddWork([&workSentinel] ()
      {
        workSentinel = 5;
      });
  EXPECT_TRUE(pool.WaitForResults(std::chrono::seconds(5)));
  EXPECT_EQ(5, workSentinel);
}

//////////////////////////////////////////////////
TEST(WorkerPool, WaitWithTimeoutThatTimesOut)
{
  std::promise<void> release;
  std::shared_future<void> released = release.get_future();

  // Declared after the objects captured by the work so that its destructor
  // joins the workers before those objects are destroyed.
  common::WorkerPool pool;

  // The worker blocks until it is released, so the wait below always times
  // out regardless of the timeout length.
  pool.AddWork([released] ()
      {
        released.wait();
      });
  EXPECT_FALSE(pool.WaitForResults(std::chrono::nanoseconds(100000)));
  release.set_value();
  EXPECT_TRUE(pool.WaitForResults(std::chrono::seconds(10)));
}

//////////////////////////////////////////////////
TEST(WorkerPool, ThingsRunInParallel)
{
  const unsigned int hc = std::thread::hardware_concurrency();
  if (2 > hc)
  {
    gzdbg << "Skipping the ThingsRunInParallel test because hardware "
           << "concurrency (" << hc << ") is too low (min: 2), making the test "
           << "less likely to succeed.\n";
    return;
  }

  std::promise<void> release;
  std::shared_future<void> released = release.get_future();
  std::atomic<int> startedCount{0};

  // Declared after the objects captured by the work so that its destructor
  // joins the workers before those objects are destroyed.
  common::WorkerPool pool;

  // Each worker blocks until released, so both can only have started if the
  // pool runs them in parallel.
  auto work = [&startedCount, released] ()
    {
      ++startedCount;
      released.wait();
    };
  pool.AddWork(work);
  pool.AddWork(work);

  const auto timeout = std::chrono::seconds(10);
  const auto deadline = std::chrono::steady_clock::now() + timeout;
  while (startedCount.load() < 2 && std::chrono::steady_clock::now() < deadline)
    std::this_thread::yield();

  const bool bothStarted = startedCount.load() == 2;

  // Release the workers before any assertion, otherwise a failure would
  // leave them blocked forever.
  release.set_value();
  EXPECT_TRUE(pool.WaitForResults(timeout));
  EXPECT_TRUE(bothStarted)
    << "pool did not run both work items in parallel within "
    << timeout.count() << " seconds";
}
