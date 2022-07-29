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

#include "gz/common/CSVStreams.hh"

#include <csv.h>

using namespace gz;
using namespace common;

class CSVIStreamIterator::Implementation
{
  public: Implementation() = default;

  public: explicit Implementation(std::istream &_stream)
    : stream(&_stream)
  {
    csv_init(&this->parser, CSV_STRICT);
  }

  public: Implementation(const Implementation &_other)
    : stream(_other.stream), row(_other.row)
  {
    if (this->stream)
    {
      csv_init(&this->parser, CSV_STRICT);
    }
  }

  public: ~Implementation()
  {
    csv_free(&this->parser);
  }

  public: void Next()
  {
    bool parsingFailed = false;
    this->row.clear();
    do
    {
      this->stream->peek();
      if (this->stream->eof())
      {
        if (this->parsing)
        {
          parsingFailed = csv_fini(
              &this->parser, OnCSVFieldSubmit,
              OnCSVRecordSubmit, this) != 0;
          this->parsing = false;
          break;
        }
        this->stream = nullptr;
        break;
      }
      if (!this->stream->getline(this->buffer, sizeof(this->buffer)))
      {
        this->stream = nullptr;
        break;
      }
      if (this->stream->good())
      {
        this->buffer[this->stream->gcount() - 1] = '\n';
      }
      this->parsing = true;
      const size_t nbytes = csv_parse(
          &this->parser, this->buffer, this->stream->gcount(),
          OnCSVFieldSubmit, OnCSVRecordSubmit, this);
      if (nbytes != static_cast<size_t>(this->stream->gcount()))
      {
        parsingFailed = true;
        break;
      }
    } while (this->parsing);

    if (parsingFailed)
    {
      try
      {
        this->stream->setstate(std::istream::failbit);
      }
      catch (const std::istream::failure &)
      {
        const char * message =
            csv_strerror(csv_error(&this->parser));
        throw std::istream::failure(message);
      }
      this->stream = nullptr;
      this->parsing = false;
    }

    if (!this->stream)
    {
      this->row.clear();
    }
  }

  public: static void OnCSVFieldSubmit(void * _data, size_t _size, void *_self)
  {
    auto data = reinterpret_cast<char *>(_data);
    auto self = reinterpret_cast<Implementation *>(_self);
    self->row.push_back(std::string(data, _size));
  }

  public: static void OnCSVRecordSubmit(int, void *_self)
  {
    auto self = reinterpret_cast<Implementation *>(_self);
    self->parsing = false;
  }

  public: std::istream *stream{nullptr};

  public: csv_parser parser{};

  public: char buffer[1024];

  public: bool parsing;

  public: std::vector<std::string> row;
};

CSVIStreamIterator::CSVIStreamIterator()
  : dataPtr(gz::utils::MakeImpl<Implementation>())
{
}

CSVIStreamIterator::CSVIStreamIterator(std::istream &_stream)
  : dataPtr(gz::utils::MakeImpl<Implementation>(_stream))
{
  this->dataPtr->Next();
}

bool CSVIStreamIterator::operator==(const CSVIStreamIterator &_other) const
{
  return this->dataPtr->stream == _other.dataPtr->stream && (
      this->dataPtr->stream == nullptr ||
      this->dataPtr->stream == _other.dataPtr->stream);
}

bool CSVIStreamIterator::operator!=(const CSVIStreamIterator &_other) const
{
  return !(*this == _other);
}

CSVIStreamIterator &CSVIStreamIterator::operator++()
{
  this->dataPtr->Next();
  return *this;
}

// NOLINTNEXTLINE(readability/casting)
CSVIStreamIterator CSVIStreamIterator::operator++(int)
{
  CSVIStreamIterator it(*this);
  this->dataPtr->Next();
  return it;
}

const std::vector<std::string> &CSVIStreamIterator::operator*() const
{
  return this->dataPtr->row;
}

const std::vector<std::string> *CSVIStreamIterator::operator->() const
{
  return &this->dataPtr->row;
}
