/*
 * Copyright (C) 2012-2014 Open Source Robotics Foundation
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
#ifndef _IGNITION_COMMON_UTIL_HH_
#define _IGNITION_COMMON_UTIL_HH_

#include <cassert>
#include <memory>
#include <string>
#include <future>

/////////////////////////////////////////////////
// Defines

/// \brief Seconds in one nano second.
#define IGN_NANO_TO_SEC 0.0000000001

/// \brief Nano seconds in one second.
#define IGN_SEC_TO_NANO 1000000000

/// \brief Nano seconds in one millisecond.
#define IGN_MS_TO_NANO 1000000

/// \brief Nano seconds in one microsecond.
#define IGN_US_TO_NANO 1000

/// \brief Speed of light.
#define IGN_SPEED_OF_LIGHT = 299792458.0

/// \brief This macro logs an error to the throw stream and throws
/// an exception that contains the file name and line number.
#define ign_throw(msg) (throw std::runtime_error(std::string("[") +\
      __FILE__ + ":" + std::to_string(__LINE__) + "] " + msg))

/// \brief Sleep for the specifed number of seconds
#define ign_sleep_s(_s) (std::this_thread::sleep_for(\
                         std::chrono::seconds(_s)))

/// \brief Sleep for the specifed number of microseconds
#define ign_sleep_us(_us) (std::this_thread::sleep_for(\
                           std::chrono::microseconds(_us)))

/// \brief Sleep for the specifed number of milliseconds
#define ign_sleep_ms(_ms) (std::this_thread::sleep_for(\
                           std::chrono::milliseconds(_ms)))

/// \brief Sleep for the specifed number of nanoseconds
#define ign_sleep_ns(_ns) (std::this_thread::sleep_for(\
                           std::chrono::nanoseconds(_ns)))

/// \brief Get the system time.
#define ign_system_time() (std::chrono::system_clock::now())

/// \brief Get the system time in seconds since epoch.
#define ign_system_time_s() (std::chrono::duration_cast<std::chrono::seconds>(\
      std::chrono::system_clock::now().time_since_epoch()).count())

/// \brief Get the system time in microseconds since epoch.
#define ign_system_time_us() (std::chrono::duration_cast<\
    std::chrono::microseconds>(\
      std::chrono::system_clock::now().time_since_epoch()).count())

/// \brief Get the system time in milliseconds since epoch.
#define ign_system_time_ms() (std::chrono::duration_cast<\
    std::chrono::milliseconds>(\
      std::chrono::system_clock::now().time_since_epoch()).count())

/// \brief Get the system time in nanoseconds since epoch.
#define ign_system_time_ns() (std::chrono::duration_cast<\
    std::chrono::nanoseconds>(\
      std::chrono::system_clock::now().time_since_epoch()).count())

/// \brief This macro defines the standard way of launching an exception
/// inside ignition.
#define ign_assert(_expr, _msg) assert((_msg, _expr))

#if defined(__GNUC__)
#define IGN_DEPRECATED(version) __attribute__((deprecated))
#define IGN_FORCEINLINE __attribute__((always_inline))
#elif defined(MSVC)
#define IGN_DEPRECATED(version) ()
#define IGN_FORCEINLINE __forceinline
#else
#define IGN_DEPRECATED(version) ()
#define IGN_FORCEINLINE
#endif

/// \def IGNITION_VISIBLE
/// Use to represent "symbol visible" if supported

/// \def IGNITION_HIDDEN
/// Use to represent "symbol hidden" if supported
#if defined _WIN32 || defined __CYGWIN__
  #ifdef BUILDING_DLL
    #ifdef __GNUC__
      #define IGNITION_VISIBLE __attribute__ ((dllexport))
    #else
      #define IGNITION_VISIBLE __declspec(dllexport)
    #endif
  #else
    #ifdef __GNUC__
      #define IGNITION_VISIBLE __attribute__ ((dllimport))
    #else
      #define IGNITION_VISIBLE __declspec(dllimport)
    #endif
  #endif
  #define IGNITION_HIDDEN
#else
  #if __GNUC__ >= 4
    #define IGNITION_VISIBLE __attribute__ ((visibility ("default")))
    #define IGNITION_HIDDEN  __attribute__ ((visibility ("hidden")))
  #else
    #define IGNITION_VISIBLE
    #define IGNITION_HIDDEN
  #endif
#endif

/// \brief Forward declarations for the common classes
namespace ignition
{
  namespace common
  {
    /// \brief A runtime error.
    typedef std::runtime_error exception;

    /// \brief Get the wall time as an ISO string: YYYY-MM-DDTHH:MM:SS.NS
    /// \return The current wall time as an ISO string.
    std::string SystemTimeISO();
  }
}
#endif
