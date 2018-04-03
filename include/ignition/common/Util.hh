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
#ifndef IGNITION_COMMON_UTIL_HH_
#define IGNITION_COMMON_UTIL_HH_

#include <cassert>
#include <memory>
#include <string>
#include <future>
#include <vector>
#include <ignition/common/Export.hh>
#include <ignition/common/Filesystem.hh>

/////////////////////////////////////////////////
// Defines

#ifdef _WIN32
# define IGN_HOMEDIR "HOMEPATH"
#else
# define IGN_HOMEDIR "HOME"
#endif

/// \brief Seconds in one nano second.
#define IGN_NANO_TO_SEC 1e-9

/// \brief Nano seconds in one second.
#define IGN_SEC_TO_NANO 1000000000

/// \brief Nano seconds in one millisecond.
#define IGN_MS_TO_NANO 1000000

/// \brief Nano seconds in one microsecond.
#define IGN_US_TO_NANO 1000

/// \brief Speed of light.
#define IGN_SPEED_OF_LIGHT = 299792458.0

/// \brief Sleep for the specifed number of seconds
#define IGN_SLEEP_S(_s) (std::this_thread::sleep_for(\
                         std::chrono::seconds(_s)))

/// \brief Sleep for the specifed number of microseconds
#define IGN_SLEEP_US(_us) (std::this_thread::sleep_for(\
                           std::chrono::microseconds(_us)))

/// \brief Sleep for the specifed number of milliseconds
#define IGN_SLEEP_MS(_ms) (std::this_thread::sleep_for(\
                           std::chrono::milliseconds(_ms)))

/// \brief Sleep for the specifed number of nanoseconds
#define IGN_SLEEP_NS(_ns) (std::this_thread::sleep_for(\
                           std::chrono::nanoseconds(_ns)))

/// \brief Get the system time.
#define IGN_SYSTEM_TIME() (std::chrono::system_clock::now())

/// \brief Get the system time in seconds since epoch.
#define IGN_SYSTEM_TIME_S() (std::chrono::duration_cast<std::chrono::seconds>(\
      std::chrono::system_clock::now().time_since_epoch()).count())

/// \brief Get the system time in microseconds since epoch.
#define IGN_SYSTEM_TIME_US() (\
    std::chrono::duration_cast<std::chrono::microseconds>(\
      std::chrono::system_clock::now().time_since_epoch()).count())

/// \brief Get the system time in milliseconds since epoch.
#define IGN_SYSTEM_TIME_MS() (\
    std::chrono::duration_cast<std::chrono::milliseconds>(\
      std::chrono::system_clock::now().time_since_epoch()).count())

/// \brief Get the system time in nanoseconds since epoch.
#define IGN_SYSTEM_TIME_NS() (\
    std::chrono::duration_cast<std::chrono::nanoseconds>(\
      std::chrono::system_clock::now().time_since_epoch()).count())

/// \brief This macro defines the standard way of launching an exception
/// inside ignition.
#define IGN_ASSERT(_expr, _msg) assert((_msg, _expr))

/// \brief Forward declarations for the common classes
namespace ignition
{
  namespace common
  {
    /// \brief Enumeration of the transform types
    enum NodeTransformType {TRANSLATE, ROTATE, SCALE, MATRIX};

    /// \brief A runtime error.
    typedef std::runtime_error exception;

    /// \brief Get the wall time as an ISO string: YYYY-MM-DDTHH:MM:SS.NS
    /// \return The current wall time as an ISO string.
    std::string IGNITION_COMMON_VISIBLE systemTimeISO();

    /// \brief Get the log path
    /// \return the log path
    std::string IGNITION_COMMON_VISIBLE logPath();

    /// \brief add path sufix to common::SystemPaths
    /// \param[in] _suffix The suffix to add.
    void IGNITION_COMMON_VISIBLE addSearchPathSuffix(
        const std::string &_suffix);

    /// \brief search for file in common::SystemPaths
    /// \param[in] _file Name of the file to find.
    /// \return The path containing the file.
    std::string IGNITION_COMMON_VISIBLE findFile(const std::string &_file);

    /// \brief search for file in common::SystemPaths
    /// \param[in] _file Name of the file to find.
    /// \param[in] _searchLocalPath True to search in the current working
    /// directory.
    /// \return The path containing the file.
    std::string IGNITION_COMMON_VISIBLE findFile(const std::string &_file,
                         bool _searchLocalPath);

    /// \brief search for a file in common::SystemPaths
    /// \param[in] _file the file name to look for.
    /// \return The path containing the file.
    std::string IGNITION_COMMON_VISIBLE findFilePath(const std::string &_file);

    /// \brief Compute the SHA1 hash of an array of bytes.
    /// \param[in] _buffer Input sequence. The permitted data types for this
    /// function are std::string and any STL container.
    /// \return The string representation (40 character) of the SHA1 hash.
    template<typename T>
    std::string sha1(const T &_buffer);

    /// \brief Compute the SHA1 hash of an array of bytes. Use std::string
    /// sha1(const T &_buffer) instead of this function
    /// \param[in] _buffer Input sequence. The permitted data types for this
    /// function are std::string and any STL container.
    /// \return The string representation (40 character) of the SHA1 hash.
    /// \sa sha1(const T &_buffer)
    std::string IGNITION_COMMON_VISIBLE sha1(
        void const *_buffer, std::size_t _byteCount);

    /// \brief Find the environment variable '_name' and return its value.
    /// \param[in] _name Name of the environment variable.
    /// \param[out] _value Value if the variable was found.
    /// \return True if the variable was found or false otherwise.
    bool IGNITION_COMMON_VISIBLE env(
        const std::string &_name, std::string &_value);

    /// \brief Get a UUID
    /// \return A UUID string
    std::string IGNITION_COMMON_VISIBLE uuid();

    /// \brief Splits a string into tokens.
    /// \param[in] _str Input string.
    /// \param[in] _delim Token delimiter.
    /// \return Vector of tokens.
    std::vector<std::string> IGNITION_COMMON_VISIBLE split(
        const std::string &_str, const std::string &_delim);

    /// \brief In place left trim
    /// \param[in,out] _s String to trim
    void IGNITION_COMMON_VISIBLE ltrim(std::string &_s);

    /// \brief In place right trim
    /// \param[in,out] _s String to trim
    void IGNITION_COMMON_VISIBLE rtrim(std::string &_s);

    /// \brief In place trim from both ends
    /// \param[in,out] _s String to trim
    void IGNITION_COMMON_VISIBLE trim(std::string &_s);

    /// \brief Copying left trim
    /// \param[in] _s String to trim
    /// \return Trimmed string
    std::string IGNITION_COMMON_VISIBLE ltrimmed(std::string _s);

    /// \brief Copying right trim
    /// \param[in] _s String to trim
    /// \return Trimmed string
    std::string IGNITION_COMMON_VISIBLE rtrimmed(std::string _s);

    /// \brief Copying trim from both ends
    /// \param[in] _s String to trim
    /// \return Trimmed string
    std::string IGNITION_COMMON_VISIBLE trimmed(std::string _s);

    /// \brief Transforms a string to its lowercase equivalent
    /// \param[in] _in String to convert to lowercase
    /// \return Lowercase equilvalent of _in.
    std::string IGNITION_COMMON_VISIBLE lowercase(const std::string &_in);

    /// \brief Transforms a string to its lowercase equivalent
    /// \param[in] _in String to convert to lowercase
    /// \return Lowercase equilvalent of _in.
    std::string IGNITION_COMMON_VISIBLE lowercase(const char *_in);

    /// \brief Replace all occurances of _key with _replacement.
    /// \param[out] _result The new string that has had _key replaced
    /// with _replacement.
    /// \param[in] _orig Original string.
    /// \param[in] _key String to replace.
    /// \param[in] _replacement The string that replaces _key.
    /// \sa  std::string replaceAll(const std::string &_orig,
    /// const std::string &_key, const std::string &_replacement)
    void IGNITION_COMMON_VISIBLE replaceAll(std::string &_result,
        const std::string &_orig,
        const std::string &_key,
        const std::string &_replacement);

    /// \brief Replace all occurances of _key with _replacement.
    /// \param[in] _orig Original string.
    /// \param[in] _key String to replace.
    /// \param[in] _replacement The string that replaces _key.
    /// \return The new string that has had _key replaced with _replacement.
    /// \sa void common::replaceAll(std::string &_result,
    /// const std::string &_orig, const std::string &_key,
    /// const std::string &_replacement)
    std::string IGNITION_COMMON_VISIBLE replaceAll(const std::string &_orig,
        const std::string &_key,
        const std::string &_replacement);
  }
}

///////////////////////////////////////////////
// Implementation of get_sha1
template<typename T>
std::string ignition::common::sha1(const T &_buffer)
{
  if (_buffer.size() == 0)
    return ignition::common::sha1(NULL, 0);
  else
  {
    return ignition::common::sha1(
        &(_buffer[0]), _buffer.size() * sizeof(_buffer[0]));
  }
}
#endif
