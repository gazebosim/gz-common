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
#ifndef IGNITION_COMMON_BASE64_HH_
#define IGNITION_COMMON_BASE64_HH_

#include <string>
#include <ignition/common/Export.hh>

namespace ignition
{
  namespace common
  {
    class IGNITION_COMMON_VISIBLE Base64
    {
      /// \brief Encode a binary string into base 64, padded with '='.
      /// \param[in] _bytesToEncode String of bytes to encode.
      /// \param[in] _len Length of _bytesToEncode.
      /// \param[out] _result Based64 string is appended to this string.
      public: static void Encode(const char *_bytesToEncode, unsigned int _len,
                                 std::string &_result);


      /// \brief Decode a base64 string.
      /// \param[in] _encodedString A base 64 encoded string.
      /// \return The decoded string.
      public: static std::string Decode(const std::string &_encodedString);
    };
  }
}
#endif
