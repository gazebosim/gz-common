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
#include <algorithm>
#include <chrono>
#include <future>
#include <thread>

#include "gz/common/Console.hh"
#include "gz/common/WorkerPool.hh"
#include "gz/utils/ExtraTestMacros.hh"

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
  common::WorkerPool pool;
  std::promise<void> release;
  std::future<void> released = release.get_future();

  // Worker waits until it is signalled to stop, therefore
  // timeout length doesn't matter here
  pool.AddWork([&released] ()
      {
        released.wait();
      });
  EXPECT_FALSE(pool.WaitForResults(std::chrono::nanoseconds(100000)));
  release.set_value();
  EXPECT_TRUE(pool.WaitForResults());
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

  common::WorkerPool pool;
  std::promise<void> release;
  std::shared_future<void> released = release.get_future();
  std::atomic<int> started_count{0};
  std::atomic<int64_t> worker_1_start{0}, worker_1_end{0}, worker_2_start{0}, worker_2_end{0};
  const auto base_time = std::chrono::steady_clock::now().time_since_epoch();
  auto sinceBase = [base_time](const std::chrono::steady_clock::time_point &t) {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        t.time_since_epoch() - base_time).count();
  };

  // Worker waits until signal
  auto work = [&](std::atomic<int64_t> &start_time, std::atomic<int64_t> &end_time) {
    start_time = sinceBase(std::chrono::steady_clock::now());
    ++started_count;
    released.wait();
    end_time = sinceBase(std::chrono::steady_clock::now());
  };

  pool.AddWork([&] { work(worker_1_start, worker_1_end); });
  pool.AddWork([&] { work(worker_2_start, worker_2_end); });

  // Wait until both workers are running, then let them finish
  const unsigned int deadline_seconds = 10;
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(deadline_seconds);
  while (started_count.load() < 2 && std::chrono::steady_clock::now() < deadline)
    std::this_thread::yield();

  const bool bothStarted = started_count.load() == 2;
  release.set_value();

  EXPECT_TRUE(pool.WaitForResults(std::chrono::seconds(10)));
  ASSERT_TRUE(bothStarted)
    << "pool did not dispatch both work items before the deadline ("
    << deadline_seconds << " seconds)";

  const int64_t overlapStart = std::max(worker_1_start.load(), worker_2_start.load());
  const int64_t overlapEnd   = std::min(worker_1_end.load(), worker_2_end.load());
  EXPECT_LT(overlapStart, overlapEnd);
}
