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
    /// \brief A single-pass row iterator on an input stream of CSV data.
    ///
    /// Similar to std::istream_iterator, this iterator parses a stream of
    /// CSV data, one row at a time.
    class GZ_COMMON_VISIBLE CSVIStreamIterator
    {
      public: using iterator_category = std::input_iterator_tag;
      public: using value_type = std::vector<std::string>;
      public: using difference_type = std::ptrdiff_t;
      public: using pointer = const value_type*;
      public: using reference = const value_type&;

      /// \brief Construct an end-of-stream iterator.
      public: CSVIStreamIterator();

      /// \brief Construct an iterator over `_stream`.
      ///
      /// The first row will be read from the underlying stream to
      /// initialize the iterator. If the read fails, the iterator
      /// becomes an end-of-stream iterator.
      ///
      /// \param[in] _stream A stream of characters to iterate.
      /// \throws std::istream::failure if a stream-specific or CSV
      /// parsing error occurs and the underlying stream exception mask
      /// allows for it.
      public: explicit CSVIStreamIterator(std::istream &_stream);

      /// \brief Read the next row from the underlying stream.
      ///
      /// If the read fails, the iterator becomes an end-of-stream iterator.
      /// If an exception is throw, using the iterator further is undefined
      /// behavior. If the iterator already is an end-of-stream iterator,
      /// behavior is undefined.
      ///
      /// \return A reference to the iterator once modified.
      /// \throws std::istream::failure if a stream-specific or CSV
      /// parsing error occurs and the underlying stream exception mask
      /// allows for it.
      public: CSVIStreamIterator &operator++();

      /// \brief Read the next row from the underlying stream.
      ///
      /// If the read fails, the iterator becomes an end-of-stream iterator.
      /// If an exception is throw, using the iterator further is undefined
      /// behavior. If the iterator already is an end-of-stream iterator,
      /// behavior is undefined.
      ///
      /// \return A copy of the iterator before modification. Note that,
      /// while an iterator copy retains its state, the underlying stream
      /// may still be advanced.
      /// \throws std::istream::failure if a stream-specific or CSV
      /// parsing error occurs and the underlying stream exception mask
      /// allows for it.
      public: CSVIStreamIterator operator++(int);

      /// \brief Check for iterator equality.
      /// \param[in] _other Iterator to compare with.
      /// \return true if both iterators are end-of-stream iterators
      /// or if both iterator wrap the same stream, false otherwise.
      public: bool operator==(const CSVIStreamIterator &_other) const;

      /// \brief Check for iterator inequality.
      /// \param[in] _other Iterator to compare with.
      /// \return true if both iterators are not equal, false otherwise.
      public: bool operator!=(const CSVIStreamIterator &_other) const;

      /// \brief Access current row.
      ///
      /// Behavior is undefined if the iterator is an end-of-stream iterator.
      ///
      /// \return reference to the current row.
      public: reference operator*() const;

      /// \brief Access current row.
      ///
      /// Behavior is undefined if the iterator is an end-of-stream iterator.
      ///
      /// \return pointer to the current row.
      public: pointer operator->() const;

      /// \brief Pointer to private data.
      private: GZ_UTILS_IMPL_PTR(dataPtr)
    };
  }
}

#endif
