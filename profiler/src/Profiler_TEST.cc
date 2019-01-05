/*
 * Copyright (C) 2018 Open Source Robotics Foundation
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

#include "ignition/common/Profiler.hh" // NOLINT(*)
#include <gtest/gtest.h> // NOLINT(*)

#include <atomic> // NOLINT(*)
#include <thread> // NOLINT(*)
#include "ignition/common/Util.hh" // NOLINT(*)
#include "ignition/common/Time.hh" // NOLINT(*)

using namespace ignition;
using namespace common;

void a_function() {
  IGN_PROFILE("a_function");
  common::Time::Sleep(common::Time(0, 100));
}

void b_function() {
  IGN_PROFILE("b_function");
  a_function();
  a_function();
  a_function();
}

void c_function() {
  IGN_PROFILE("c_function");
  b_function();
  b_function();
  b_function();
}

/////////////////////////////////////////////////
TEST(Profiler, Profiler)
{
  std::atomic<bool> running {true};

  auto thread_fcn = [&running](const std::string& name){
    IGN_PROFILE_THREAD_NAME(name.c_str());
    while (running)
    {
      IGN_PROFILE("Loop");
      c_function();
    }
  };

  auto t1 = std::thread(thread_fcn, "thread1");
  auto t2 = std::thread(thread_fcn, "thread2");
  auto t3 = std::thread(thread_fcn, "thread3");
  auto t4 = std::thread(thread_fcn, "thread4");
  auto t5 = std::thread(thread_fcn, "thread5");

  IGN_PROFILE_THREAD_NAME("main thread");
  for (size_t ii = 0; ii < 100; ++ii) {
    IGN_PROFILE("Loop");
    common::Time::Sleep(common::Time(1, 0));
    IGN_PROFILE_LOG_TEXT("Loop");
  }
  running = false;
  t1.join();
  t2.join();
  t3.join();
  t4.join();
  t5.join();
}
