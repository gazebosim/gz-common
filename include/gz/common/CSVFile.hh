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
#ifndef GZ_COMMON_CSVFILE_HH_
#define GZ_COMMON_CSVFILE_HH_

#include <fstream>
#include <istream>
#include <string>
#include <vector>

#include <gz/utils/ImplPtr.hh>

namespace gz
{
  namespace common
  {
    class CSVStreamIterator
    {
      public: using iterator_category = std::input_iterator_tag;
      public: using value_type = std::vector<std::string>;
      public: using difference_type = int;
      public: using pointer = const value_type*;
      public: using reference = const value_type&;

      public: CSVStreamIterator();

      public: CSVStreamIterator(std::istream &_stream);

      public: CSVStreamIterator(CSVStreamIterator &&_other);

      public: CSVStreamIterator(const CSVStreamIterator &_other);

      public: std::istream *Stream();

      public: void Next();
      public: CSVStreamIterator &operator++();
      public: CSVStreamIterator operator++(int);

      public: bool operator==(const CSVStreamIterator &_other) const;
      public: bool operator!=(const CSVStreamIterator &_other) const;

      public: reference operator*() const;
      public: pointer operator->() const;

      /// \brief Pointer to private data.
      private: GZ_UTILS_UNIQUE_IMPL_PTR(dataPtr)
    };

    class CSVFile
    {
      public: class Cursor
      {
        public: Cursor(const CSVFile &_file);

        public: std::ifstream &Stream();

        public: CSVStreamIterator begin() const;

        public: CSVStreamIterator end() const;

        /// \brief Pointer to private data.
        private: GZ_UTILS_UNIQUE_IMPL_PTR(dataPtr)
      };

      public: explicit CSVFile(
        const std::string &_path,
        bool _has_header = true);

      public: const std::string &Path() const;

      public: const std::vector<std::string> &Header() const;

      public: Cursor Data() const;

      public: size_t NumRows() const;

      public: size_t NumColumns() const;

      /// \brief Pointer to private data.
      private: GZ_UTILS_UNIQUE_IMPL_PTR(dataPtr)
    };
  }
}

#endif
