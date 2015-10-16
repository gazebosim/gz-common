/*
 * Copyright (C) 2012-2014 Open Source Robotics Foundation
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
#include <time.h>
#include <sys/time.h>
#include <ctime>
#include <chrono>

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

#include <math.h>

#include <ignition/common/Util.hh>
#include <ignition/common/Console.hh>
#include <ignition/common/Time.hh>

using namespace ignition;
using namespace common;

Time Time::wallTime;

const Time Time::Zero = common::Time(0, 0);
const int32_t Time::nsInSec = 1000000000L;
const int32_t Time::nsInMs = 1000000;


/////////////////////////////////////////////////
Time::Time()
  : sec(0), nsec(0)
{
}

/////////////////////////////////////////////////
Time::Time(const Time &_time)
: sec(_time.sec), nsec(_time.nsec)
{
}

/////////////////////////////////////////////////
Time::Time(int32_t _sec, int32_t _nsec)
: sec(_sec), nsec(_nsec)
{
  this->Correct();
}

/////////////////////////////////////////////////
Time::Time(double _time)
{
  this->Set(_time);
}

/////////////////////////////////////////////////
Time::~Time()
{
}

/////////////////////////////////////////////////
const Time &Time::SystemTime()
{
  auto duration = ign_system_time().time_since_epoch();
  wallTime.sec = std::chrono::duration_cast<std::chrono::seconds>(
      duration).count();
  wallTime.nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(
      duration).count() - wallTime.sec * IGN_SEC_TO_NANO;

  return wallTime;
}

/////////////////////////////////////////////////
void Time::Set(int32_t _sec, int32_t _nsec)
{
  this->sec = _sec;
  this->nsec = _nsec;

  this->Correct();
}

/////////////////////////////////////////////////
void Time::Set(double _seconds)
{
  this->sec = (int32_t)(floor(_seconds));
  this->nsec = (int32_t)(round((_seconds - this->sec) * IGN_SEC_TO_NANO));
  this->Correct();
}

/////////////////////////////////////////////////
double Time::Double() const
{
  return static_cast<double>(this->sec + this->nsec * IGN_NANO_TO_SEC);
}

/////////////////////////////////////////////////
float Time::Float() const
{
  return static_cast<float>(this->sec + this->nsec * IGN_NANO_TO_SEC);
}

/////////////////////////////////////////////////
Time Time::Sleep(const common::Time &_time)
{
  Time result;

  struct timespec clockResolution;
#ifdef __MACH__
  clockResolution.tv_sec = 1 / sysconf(_SC_CLK_TCK);
#else
  // get clock resolution, skip sleep if resolution is larger then
  // requested sleep time
  clock_getres(CLOCK_REALTIME, &clockResolution);
#endif

  if (_time >= Time(clockResolution.tv_sec, clockResolution.tv_nsec))
  {
    struct timespec interval;
    struct timespec remainder;
    interval.tv_sec = _time.sec;
    interval.tv_nsec = _time.nsec;

    // Sleeping for negative time doesn't make sense
    if (interval.tv_sec < 0)
    {
      ignerr << "Cannot sleep for negative time[" << _time << "]\n";
      return result;
    }

    // This assert conforms to the manpage for nanosleep
    if (interval.tv_nsec < 0 || interval.tv_nsec > 999999999)
    {
      ignerr << "Nanoseconds of [" << interval.tv_nsec
            << "] must be in the range0 to 999999999.\n";
      return result;
    }

#ifdef __MACH__
    if (nanosleep(&interval, &remainder) == -1)
    {
#else
    if (clock_nanosleep(CLOCK_REALTIME, 0, &interval, &remainder) == -1)
    {
#endif
      result.sec = remainder.tv_sec;
      result.nsec = remainder.tv_nsec;
    }
  }
  else
  {
    /// \TODO Make this a gzlog
    ignwarn << "Sleep time is larger than clock resolution, skipping sleep\n";
  }

  return result;
}

/////////////////////////////////////////////////
Time &Time::operator=(const Time &_time)
{
  this->sec = _time.sec;
  this->nsec = _time.nsec;

  return *this;
}

/////////////////////////////////////////////////
Time Time::operator+(const Time &_time) const
{
  Time t(this->sec + _time.sec, this->nsec + _time.nsec);
  t.Correct();

  return t;
}

/////////////////////////////////////////////////
const Time &Time::operator+=(const Time &_time)
{
  this->sec += _time.sec;
  this->nsec += _time.nsec;
  this->Correct();
  return *this;
}

/////////////////////////////////////////////////
Time Time::operator-(const Time &_time) const
{
  Time t(this->sec - _time.sec, this->nsec - _time.nsec);
  t.Correct();
  return t;
}

/////////////////////////////////////////////////
const Time &Time::operator-=(const Time &_time)
{
  this->sec -= _time.sec;
  this->nsec -= _time.nsec;
  this->Correct();
  return *this;
}

/////////////////////////////////////////////////
Time Time::operator*(const Time &_time) const
{
  Time t(this->sec * _time.sec, this->nsec * _time.nsec);
  t.Correct();
  return t;
}

/////////////////////////////////////////////////
const Time &Time::operator*=(const Time &_time)
{
  this->Set(this->sec * _time.sec, this->nsec * _time.nsec);
  this->Correct();
  return *this;
}

/////////////////////////////////////////////////
Time Time::operator/(const Time &_time) const
{
  Time result(*this);

  if (_time.sec == 0 && _time.nsec == 0)
    ignerr << "Time divide by zero\n";
  else
    result.Set(this->Double() / _time.Double());

  return result;
}

/////////////////////////////////////////////////
const Time &Time::operator/=(const Time &_time)
{
  *this = *this / _time;
  return *this;
}

/////////////////////////////////////////////////
bool Time::operator==(const Time &_time) const
{
  return this->sec == _time.sec && this->nsec == _time.nsec;
}

/////////////////////////////////////////////////
bool Time::operator==(double _time) const
{
  return *this == Time(_time);
}

/////////////////////////////////////////////////
bool Time::operator!=(const Time &_time) const
{
  return !(*this == _time);
}

/////////////////////////////////////////////////
bool Time::operator!=(double _time) const
{
  return !(*this == _time);
}

/////////////////////////////////////////////////
bool Time::operator<(const Time &_time) const
{
  return this->sec < _time.sec ||
    (this->sec == _time.sec && this->nsec < _time.nsec);
}

/////////////////////////////////////////////////
bool Time::operator<(double _time) const
{
  return *this < Time(_time);
}

/////////////////////////////////////////////////
bool Time::operator<=(const Time &_time) const
{
  return !(_time < *this);
}

/////////////////////////////////////////////////
bool Time::operator<=(double _time) const
{
  return *this <= Time(_time);
}

/////////////////////////////////////////////////
bool Time::operator>(const Time &_time) const
{
  return _time < *this;
}

/////////////////////////////////////////////////
bool Time::operator>(double _time) const
{
  return *this > Time(_time);
}

/////////////////////////////////////////////////
bool Time::operator>=(const Time &_time) const
{
  return !(*this < _time);
}

/////////////////////////////////////////////////
bool Time::operator>=(double _time) const
{
  return *this >= Time(_time);
}
