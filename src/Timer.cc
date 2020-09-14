/*
 * Copyright (C) 2016 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <ignition/common/Timer.hh>

using namespace ignition;
using namespace common;

//////////////////////////////////////////////////
Timer::Timer()
  : running(false)
{
}

//////////////////////////////////////////////////
Timer::~Timer()
{
}

//////////////////////////////////////////////////
void Timer::Start()
{
  this->start = std::chrono::steady_clock::now();
  this->running = true;
}

//////////////////////////////////////////////////
void Timer::Stop()
{
  this->stop = std::chrono::steady_clock::now();
  this->running = false;
}

//////////////////////////////////////////////////
bool Timer::Running() const
{
  return this->running;
}

//////////////////////////////////////////////////
Time Timer::Elapsed() const
{
  if (this->running)
  {
    std::chrono::steady_clock::time_point currentTime;
    currentTime = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = currentTime - this->start;
#ifndef _WIN32
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
    return Time(diff.count());
#ifndef _WIN32
# pragma GCC diagnostic pop
#endif
  }
  else
  {
    std::chrono::duration<double> diff = this->stop - this->start;
#ifndef _WIN32
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
    return Time(diff.count());
#ifndef _WIN32
# pragma GCC diagnostic pop
#endif
  }
}

//////////////////////////////////////////////////
double Timer::ElapsedTime() const
{
  if (this->running)
  {
    std::chrono::steady_clock::time_point currentTime;
    currentTime = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = currentTime - this->start;
    return diff.count();
  }
  else
  {
    std::chrono::duration<double> diff = this->stop - this->start;
    return diff.count();
  }
}
