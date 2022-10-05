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

#include <atomic>
#include <thread>

#include "gz/common/Console.hh"
#include "gz/common/Profiler.hh"
#include "gz/common/SignalHandler.hh"
#include "gz/common/Time.hh"

std::atomic<bool> running;

void task1()
{
  // An example of using scope-based profiling.
  IGN_PROFILE("task1");
  {
    IGN_PROFILE("this");
    {
      IGN_PROFILE("is");
      {
        IGN_PROFILE("a");
        {
          IGN_PROFILE("deep");
          {
            IGN_PROFILE("stack");
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
          }
        }
      }
    }
  }
}

void task2()
{
  // An example of using scope-based profiling.
  IGN_PROFILE("task2");
  {
    IGN_PROFILE("a");
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
  }
  {
    IGN_PROFILE("b");
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
  }
  {
    IGN_PROFILE("c");
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
  }
  {
    IGN_PROFILE("d");
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
  }
}

void task3()
{
  IGN_PROFILE("task3");

  // An example of using start/stop profiling.
  IGN_PROFILE_BEGIN("a");
  std::this_thread::sleep_for(std::chrono::milliseconds(2));
  IGN_PROFILE_END();

  IGN_PROFILE_BEGIN("b");
  std::this_thread::sleep_for(std::chrono::milliseconds(2));
  IGN_PROFILE_END();

  IGN_PROFILE_BEGIN("c");
  std::this_thread::sleep_for(std::chrono::milliseconds(2));
  IGN_PROFILE_END();

  IGN_PROFILE_BEGIN("d");
  std::this_thread::sleep_for(std::chrono::milliseconds(2));
  IGN_PROFILE_END();
}

void thread(const char *_thread_name)
{
  // Sets the name of the thread to appear in the UI
  IGN_PROFILE_THREAD_NAME(_thread_name);
  while (running)
  {
    IGN_PROFILE("Loop");
    // Execute some arbitrary tasks
    for (size_t ii = 0; ii < 10; ++ii)
    {
      task1();
    }
    task2();
    task3();
  }
}

void sigHandler(int _signal)
{
  ignmsg << "Received signal: " << _signal << std::endl;
  running = false;
}

int main(int argc, char** argv)
{
  gz::common::Console::SetVerbosity(4);

  IGN_PROFILE_THREAD_NAME("main");
  running = true;

  // Connect signal handler
  gz::common::SignalHandler handler;
  handler.AddCallback(sigHandler);

  // Spin up a couple threads with interesting content.
  auto t1 = std::thread([](){thread("thread1");});
  auto t2 = std::thread([](){thread("thread2");});
  auto t3 = std::thread([](){thread("thread3");});

  ignmsg << "All threads started, now connect with visualizer" << std::endl;

  size_t ii = 0;
  while(running)
  {
    IGN_PROFILE("main loop");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Throttle to once per second
    if (ii++ % 10 == 0)
    {
      // Example of logging text to the remotery console.
      IGN_PROFILE_LOG_TEXT("main loop done");
    }
  }

  t3.join();
  t2.join();
  t1.join();
}
