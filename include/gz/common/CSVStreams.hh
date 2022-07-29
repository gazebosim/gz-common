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
#ifndef GZ_COMMON_CSVSTREAMS_HH_
#define GZ_COMMON_CSVSTREAMS_HH_

#include <istream>
#include <string>
#include <vector>

#include <gz/common/Export.hh>

#include <gz/utils/ImplPtr.hh>

namespace gz
{
  namespace common
  {
    class GZ_COMMON_VISIBLE CSVIStreamIterator
    {
      public: using iterator_category = std::input_iterator_tag;
      public: using value_type = std::vector<std::string>;
      public: using difference_type = std::ptrdiff_t;
      public: using pointer = const value_type*;
      public: using reference = const value_type&;

      public: CSVIStreamIterator();

      public: CSVIStreamIterator(std::istream &_stream);

      public: CSVIStreamIterator &operator++();
      public: CSVIStreamIterator operator++(int);

      public: bool operator==(const CSVIStreamIterator &_other) const;
      public: bool operator!=(const CSVIStreamIterator &_other) const;

      public: reference operator*() const;
      public: pointer operator->() const;

      /// \brief Pointer to private data.
      private: GZ_UTILS_IMPL_PTR(dataPtr)
    };
  }
}

#endif
