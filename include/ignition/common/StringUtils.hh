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
#ifndef IGNITION_COMMON_STRINGUTILS_HH_
#define IGNITION_COMMON_STRINGUTILS_HH_

#include <string>
#include <vector>

#include <ignition/common/System.hh>

namespace ignition
{
  namespace common
  {
    /// \brief split at a one character delimiter to get a vector of something
    /// \param[in] _orig The string to split
    /// \param[in] _delim a character to split the string at
    /// \returns vector of split pieces of the string excluding the delimiter
    // cppcheck-suppress constStatement
    IGNITION_COMMON_VISIBLE
    std::vector<std::string> Split(const std::string &_orig, char _delim);

    /// \brief return true if string starts with another string
    /// \param[in] _s1 the string to check
    /// \param[in] _s2 the possible prefix
    /// \returns true if _s1 starts with _s2
    // cppcheck-suppress constStatement
    IGNITION_COMMON_VISIBLE
    bool StartsWith(const std::string &_s1, const std::string &_s2);

    /// \brief return true if string ends with another string
    /// \param[in] _s1 the string to check
    /// \param[in] _s2 the possible suffix
    /// \returns  true if _s1 ends with _s2
    // cppcheck-suppress constStatement
    IGNITION_COMMON_VISIBLE
    bool EndsWith(const std::string &_s1, const std::string &_s2);
  }
}

#endif
