/*
 * Copyright (C) 2022 Open Source Robotics Foundation
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
#ifndef GZ_COMMON_IO_HH_
#define GZ_COMMON_IO_HH_

#include <sstream>
#include <string>

namespace gz
{
  namespace common
  {
    template <typename T>
    struct IO {
      static T ReadFrom(std::istringstream &_istream)
      {
        T value;
        _istream >> value;
        return value;
      }

      static T ReadFrom(const std::string &_string)
      {
        std::istringstream stream{_string};
        return ReadFrom(stream);
      }
    };

    template<>
    struct IO<std::string> {
      static std::string ReadFrom(std::istringstream &_istream)
      {
        std::string value;
        _istream >> value;
        return value;
      }

      static std::string ReadFrom(const std::string &_string)
      {
        return _string;
      }
    };
  }
}

#endif
