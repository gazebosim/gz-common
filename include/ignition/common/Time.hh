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
#ifndef IGNITION_COMMON_TIME_HH_
#define IGNITION_COMMON_TIME_HH_

#include <string>
#include <iostream>

#include <ignition/common/Export.hh>
#include <ignition/common/Util.hh>

namespace ignition
{
  namespace common
  {
    /// \class Time Time.hh common/common.hh
    /// \brief A Time class, can be used to hold wall- or sim-time.
    ///        stored as sec and nano-sec.
    class IGNITION_COMMON_VISIBLE Time
    {
      /// \brief A static zero time variable set to common::Time(0, 0).
      public: static const Time Zero;

      /// \enum Format options
      /// \brief Options for formatting time as a string.
      public: enum class FormatOption
      {
        /// \brief Days
        DAYS = 0,
        /// \brief Hours
        HOURS = 1,
        /// \brief Minutes
        MINUTES = 2,
        /// \brief Seconds
        SECONDS = 3,
        /// \brief Milliseconds
        MILLISECONDS = 4
      };

      /// \brief Constructors
      public: IGN_DEPRECATED(4) Time();

      /// \brief Copy constructor
      /// \param[in] time Time to copy
      public: IGN_DEPRECATED(4) Time(const Time &_time);

      /// \brief Constructor
      /// \param[in] _tv Time to initialize to
      public: explicit IGN_DEPRECATED(4) Time(const struct timespec &_tv);

      /// \brief Constructor
      /// \param[in] _sec Seconds
      /// \param[in] _nsec Nanoseconds
      public: IGN_DEPRECATED(4) Time(int32_t _sec, int32_t _nsec);

      /// \brief Constuctor
      /// \param[in] _time Time in double format sec.nsec
      public: explicit IGN_DEPRECATED(4) Time(double _time);

      /// \brief Destructor
      public: virtual ~Time();

      /// \brief Get the wall time
      /// \return the current time
      public: static const IGN_DEPRECATED(4) Time &SystemTime();

      /// \brief Set to sec and nsec
      /// \param[in] _sec Seconds
      /// \param[in] _nsec Nanoseconds
      public: void Set(int32_t _sec, int32_t _nsec);

      /// \brief Set to seconds
      /// \param[in] _seconds Number of seconds
      public: void Set(double _seconds);

      /// \brief Get the time as a double
      /// \return Time as a double in seconds
      public: double Double() const;

      /// \brief Get the time as a float
      /// \return Time as a float in seconds
      public: float Float() const;

      /// \brief Sleep for the specified time.
      /// \param[in] _time Sleep time
      /// \return On OSX and Linux the return value is the time slept,
      /// as reported by `nanosleep` for OSX and `clock_nanosleep` for Linux.
      /// The return value does not include time spent before and after the
      /// actual call to the system's sleep function.
      ///
      /// On Windows the return value is always common::Time::Zero.
      public: static Time IGN_DEPRECATED(4) Sleep(const common::Time &_time);

      /// \brief Get the time as a string formatted as "DD hh:mm:ss.mmm", with
      /// the option to choose the start/end.
      /// \param[in] _start Start point.
      /// \param[in] _end End point.
      /// \return String representing time.
      public: std::string FormattedString(
          FormatOption _start = FormatOption::DAYS,
          FormatOption _end = FormatOption::MILLISECONDS) const;

      /// \brief Assignment operator
      /// \param[in] _time the new time
      /// \return a reference to this instance
      public: Time &operator=(const Time &_time);

      /// \brief Addition operators
      /// \param[in] _time The time to add
      /// \return a Time instance
      public: Time operator+(const Time &_time) const;

      /// \brief Addition assignemtn operator
      /// \param[in] _time The time to add
      /// \return a Time instance
      public: const Time &operator +=(const Time &_time);

      /// \brief Subtraction operator
      /// \param[in] _tv The time to subtract
      /// \return a Time instance
      public: Time operator -(const struct timespec &_tv) const;

      /// \brief Subtraction assignment operator
      /// \param[in] _tv The time to subtract
      /// \return a Time instance
      public: const Time &operator -=(const struct timespec &_tv);

      /// \brief Subtraction operator
      /// \param[in] _time The time to subtract
      /// \return a Time instance
      public: Time operator -(const Time &_time) const;

      /// \brief Subtraction assignment operator
      /// \param[in] _time The time to subtract
      /// \return a reference to this instance
      public: const Time &operator -=(const Time &_time);

      /// \brief Multiplication operator
      /// \param[in] _tv the scaling duration
      /// \return Time instance
      public: Time operator *(const struct timespec &_tv) const;

      /// \brief Multiplication assignment operator
      /// \param[in] _tv the scaling duration
      /// \return a reference to this instance
      public: const Time &operator *=(const struct timespec &_tv);

      /// \brief Multiplication operators
      /// \param[in] _time the scaling factor
      /// \return a scaled Time instance
      public: Time operator *(const Time &_time) const;

      /// \brief Multiplication operators
      /// \param[in] _time scale factor
      /// \return a scaled Time instance
      public: const Time &operator *=(const Time &_time);

      /// \brief Division assignment operator
      /// \param[in] _tv a divisor
      /// \return a Time instance
      public: const Time &operator /=(const struct timespec &_tv);

      /// \brief Division operator
      /// \param[in] _time the divisor
      /// \return a Time instance
      public: Time operator /(const Time &_time) const;

      /// \brief Division assignment operator
      /// \param[in] time the divisor
      /// \return a Time instance
      public: const Time &operator /=(const Time &time);

      /// \brief Equal to operator
      /// \param[in] _time the time to compare to
      /// \return true if values are the same, false otherwise
      public: bool operator==(const Time &_time) const;

      /// \brief Equal to operator
      /// \param[in] _time the time to compare to
      /// \return true if values are the same, false otherwise
      public: bool operator==(double _time) const;

      /// \brief Equal to operator
      /// \param[in] _time the time to compare to
      /// \return true if values are the same, false otherwise
      public: bool operator!=(const Time &_time) const;

      /// \brief Equal to operator
      /// \param[in] _time the time to compare to
      /// \return true if values are the same, false otherwise
      public: bool operator!=(double _time) const;

      /// \brief Less than operator
      /// \param[in] _time the time to compare with
      /// \return true if time is shorter than this, false otherwise
      public: bool operator<(const Time &_time) const;

      /// \brief Less than operator
      /// \param[in] _time the time to compare with
      /// \return true if time is shorter than this, false otherwise
      public: bool operator<(double _time) const;

      /// \brief Less than or equal to operator
      /// \param[in] _time the time to compare with
      /// \return true if time is shorter than or equal to this, false otherwise
      public: bool operator<=(const Time &_time) const;

      /// \brief Less than or equal to operator
      /// \param[in] _time the time to compare with
      /// \return true if time is shorter than or equal to this, false otherwise
      public: bool operator<=(double _time) const;

      /// \brief Greater than operator
      /// \param[in] _tv the time to compare with
      /// \return true if time is greater than this, false otherwise
      public: bool operator>(const struct timespec &_tv) const;

      /// \brief Greater than operator
      /// \param[in] _time the time to compare with
      /// \return true if time is greater than this, false otherwise
      public: bool operator>(const Time &_time) const;

      /// \brief Greater than operator
      /// \param[in] _time the time to compare with
      /// \return true if time is greater than this, false otherwise
      public: bool operator>(double _time) const;

      /// \brief Greater than or equal operator
      /// \param[in] _time the time to compare with
      /// \return true if time is greater than or equal to this, false otherwise
      public: bool operator>=(const Time &_time) const;

      /// \brief Greater than or equal operator
      /// \param[in] _tv the time to compare with
      /// \return true if tv is greater than or equal to this, false otherwise
      public: bool operator>=(const struct timespec &_tv) const;

      /// \brief Greater than or equal operator
      /// \param[in] _time the time to compare with
      /// \return true if time is greater than or equal to this, false otherwise
      public: bool operator>=(double _time) const;

      /// \brief Stream insertion operator
      /// \param[in] _out the output stream
      /// \param[in] _time time to write to the stream
      /// \return the output stream
      public: friend std::ostream &operator<<(std::ostream &_out,
                  const ignition::common::Time &_time)
              {
                _out << _time.sec << " " << _time.nsec;
                return _out;
              }

      /// \brief Stream extraction operator
      /// \param[in] _in the input stream
      /// \param[in] _time time to read from to the stream
      /// \return the input stream
      public: friend std::istream &operator>>(std::istream &_in,
                                              ignition::common::Time &_time)
              {
                // Skip white spaces
                _in.setf(std::ios_base::skipws);
                _in >> _time.sec >> _time.nsec;
                return _in;
              }

      /// \brief Seconds
      public: int32_t sec;

      /// \brief Nanoseconds
      public: int32_t nsec;

      /// \brief a singleton value of the last GetWallTime() value
      private: static Time wallTime;

      /// \brief Correct the time so that small additions/substractions
      /// preserve the internal seconds and nanoseconds separation
      private: inline void Correct()
               {
                 // In the case sec and nsec have different signs, normalize
                 if (this->sec > 0 && this->nsec < 0)
                 {
                   int32_t n = abs(this->nsec / this->nsInSec) + 1;
                   this->sec -= n;
                   this->nsec += n * this->nsInSec;
                 }
                 if (this->sec < 0 && this->nsec > 0)
                 {
                   int32_t n = abs(this->nsec / this->nsInSec) + 1;
                   this->sec += n;
                   this->nsec -= n * this->nsInSec;
                 }

                 // Make any corrections
                 this->sec += this->nsec / this->nsInSec;
                 this->nsec = this->nsec % this->nsInSec;
               }

      /// \brief Constant multiplier to convert from nanoseconds to seconds.
      private: static const int32_t nsInSec;

      /// \brief Constant multiplier to convert from nanoseconds to
      /// milliseconds.
      private: static const int32_t nsInMs;

      private: static struct timespec clockResolution;
    };
  }
}
#endif
