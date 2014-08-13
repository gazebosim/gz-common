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

/////////////////////////////////////////////////
// Defines

/// \brief This macro logs an error to the throw stream and throws
/// an exception that contains the file name and line number.
#define ignthrow(msg) (throw std::runtime_error(std::string("[") +\
      __FILE__ + ":" + std::to_string(__LINE__) + "] " + msg))


/// \brief This macro defines the standard way of launching an exception
/// inside ignition.
#define ignassert(_expr, _msg) assert((_msg, _expr))

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
    class Time;

    /// \brief Speed of light.
    static const double SpeedOfLight = 299792458;

    /// \brief A runtime error.
    typedef std::runtime_error exception;
  }
}
#endif
