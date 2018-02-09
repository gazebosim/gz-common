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

#include <ctime>
#include <chrono>
#include <iomanip>

#ifdef _WIN32

  #include <Windows.h>
  #include <Winsock2.h>
  #include <cstdint>
// As of MSVS2015 (which contains MSVC++ ver 19), timespec is provided in time.h
// _MSC_VER contains the the version of VC++ multiplied by 100.
#if defined(_MSC_VER) && _MSC_VER < 1900
  struct timespec
  {
    int64_t tv_sec;
    int64_t tv_nsec;
  };
#else

  #include <time.h>
#endif

#else
  #include <unistd.h>
  #include <sys/time.h>

  #include <time.h> // NOLINT (false positive for multiple includes)
#endif


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

struct timespec Time::clockResolution;
const Time Time::Zero = common::Time(0, 0);
const int32_t Time::nsInSec = 1000000000L;
const int32_t Time::nsInMs = 1000000;


/////////////////////////////////////////////////
Time::Time()
  : sec(0), nsec(0)
{
}

/////////////////////////////////////////////////
Time::Time(const struct timespec &_tv)
{
  this->sec = _tv.tv_sec;
  this->nsec = _tv.tv_nsec;
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
  auto duration = IGN_SYSTEM_TIME().time_since_epoch();
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

  if (_time >= clockResolution)
  {
    struct timespec interval;
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

#ifdef _WIN32
    // Borrowed from roscpp_core/rostime/src/time.cpp
    HANDLE timer = NULL;
    LARGE_INTEGER sleepTime;
    sleepTime.QuadPart = -
      static_cast<int64_t>(interval.tv_sec)*10000000LL -
      static_cast<int64_t>(interval.tv_nsec) / 100LL;

    timer = CreateWaitableTimer(NULL, TRUE, NULL);
    if (timer == NULL)
    {
      ignerr << "Unable to create waitable timer. Sleep will be incorrect.\n";
      return result;
    }

    if (!SetWaitableTimer (timer, &sleepTime, 0, NULL, NULL, 0))
    {
      ignerr << "Unable to use waitable timer. Sleep will be incorrect.\n";
      return result;
    }

    if (WaitForSingleObject (timer, INFINITE) != WAIT_OBJECT_0)
    {
      ignerr <<
        "Unable to wait for a single object. Sleep will be incorrect.\n";
      return result;
    }

    result.sec = 0;
    result.nsec = 0;
#else
    struct timespec remainder;
# ifdef __MACH__
    if (nanosleep(&interval, &remainder) == -1)
    {
      result.sec = remainder.tv_sec;
      result.nsec = remainder.tv_nsec;
    }
# else
    if (clock_nanosleep(CLOCK_REALTIME, 0, &interval, &remainder) == -1)
    {
      result.sec = remainder.tv_sec;
      result.nsec = remainder.tv_nsec;
    }
# endif
#endif
  }
  else
  {
    ignlog << "Sleep time is larger than clock resolution, skipping sleep\n";
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

/////////////////////////////////////////////////
bool Time::operator>=(const struct timespec &_tv) const
{
  return *this >= Time(_tv);
}

/////////////////////////////////////////////////
std::string Time::FormattedString(FormatOption _start, FormatOption _end) const
{
  if (_end < _start)
  {
    ignwarn << "End can't come before start, using same end and start."
        << std::endl;
    _end = _start;
  }

  std::ostringstream stream;
  unsigned int s, msec;

  stream.str("");

  // Get seconds
  s = this->sec;

  // Get milliseconds
  msec = this->nsec / nsInMs;

  // Get seconds from milliseconds
  int seconds = msec / 1000;
  msec -= seconds * 1000;
  s += seconds;

  // Days
  if (_start <= FormatOption::DAYS)
  {
    unsigned int day = s / 86400;
    s -= day * 86400;
    stream << std::setw(2) << std::setfill('0') << day;
  }

  // Hours
  if (_end >= FormatOption::HOURS)
  {
    if (_start <  FormatOption::HOURS)
      stream << " ";

    if (_start <=  FormatOption::HOURS)
    {
      unsigned int hour = s / 3600;
      s -= hour * 3600;
      stream << std::setw(2) << std::setfill('0') << hour;
    }
  }

  // Minutes
  if (_end >= FormatOption::MINUTES)
  {
    if (_start < FormatOption::MINUTES)
      stream << ":";

    if (_start <= FormatOption::MINUTES)
    {
      unsigned int min = s / 60;
      s -= min * 60;
      stream << std::setw(2) << std::setfill('0') << min;
    }
  }

  // Seconds
  if (_end >= FormatOption::SECONDS)
  {
    if (_start < FormatOption::SECONDS)
      stream << ":";

    if (_start <= FormatOption::SECONDS)
    {
      stream << std::setw(2) << std::setfill('0') << s;
    }
  }

  // Milliseconds
  if (_end >= FormatOption::MILLISECONDS)
  {
    if (_start < FormatOption::MILLISECONDS)
      stream << ".";
    else
      msec = msec + s * 1000;

    if (_start <= FormatOption::MILLISECONDS)
    {
      stream << std::setw(3) << std::setfill('0') << msec;
    }
  }

  return stream.str();
}
