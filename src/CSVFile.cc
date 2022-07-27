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

#include "gz/common/CSVFile.hh"

#include "gz/common/Filesystem.hh"
#include "gz/common/StringUtils.hh"

#include <algorithm>
#include <regex>

using namespace gz;
using namespace common;

class CSVStreamIterator::Implementation
{
  public: Implementation() = default;

  public: Implementation(const Implementation &_other) = default;

  public: Implementation(std::istream *_stream) : stream(_stream)
  {
  }

  public: std::vector<std::string> row{};

  public: std::istream *stream{nullptr};

  public: const std::regex pattern{
    R"#(("([^"]*)"|[^,]*)(?:,|$))#"};
};

CSVStreamIterator::CSVStreamIterator()
  : dataPtr(gz::utils::MakeUniqueImpl<Implementation>())
{
}

CSVStreamIterator::CSVStreamIterator(std::istream &_stream)
  : dataPtr(gz::utils::MakeUniqueImpl<Implementation>(&_stream))
{
  this->Next();
}

CSVStreamIterator::CSVStreamIterator(CSVStreamIterator &&_other)
  : dataPtr(std::move(_other.dataPtr))
{
}

CSVStreamIterator::CSVStreamIterator(const CSVStreamIterator &_other)
  : dataPtr(gz::utils::MakeUniqueImpl<Implementation>(*_other.dataPtr))
{
}

bool CSVStreamIterator::operator==(const CSVStreamIterator &_other) const
{
  return this->dataPtr->stream == _other.dataPtr->stream && (
      this->dataPtr->stream == nullptr ||
      this->dataPtr->stream->tellg() == _other.dataPtr->stream->tellg());
}

bool CSVStreamIterator::operator!=(const CSVStreamIterator &_other) const
{
  return !(*this == _other);
}

std::istream *CSVStreamIterator::Stream()
{
  return this->dataPtr->stream;
}

void CSVStreamIterator::Next()
{
  if (this->dataPtr->stream)
  {
    std::string line;
    this->dataPtr->row.clear();
    if (std::getline(*this->dataPtr->stream, line))
    {
      auto it = std::sregex_iterator(
          line.begin(), line.end(),
          this->dataPtr->pattern);
      for (; it != std::sregex_iterator(); ++it)
      {
        this->dataPtr->row.push_back(Trim(it->str(1), '"'));
      }
    }
    else
    {
      this->dataPtr->stream = nullptr;
    }
  }
}

CSVStreamIterator &CSVStreamIterator::operator++()
{
  this->Next();
  return *this;
}

CSVStreamIterator CSVStreamIterator::operator++(int)
{
  CSVStreamIterator it(*this);
  this->Next();
  return it;
}

const std::vector<std::string> &CSVStreamIterator::operator*() const
{
  return this->dataPtr->row;
}

const std::vector<std::string> *CSVStreamIterator::operator->() const
{
  return &this->dataPtr->row;
}

class CSVFile::Cursor::Implementation
{
  public: Implementation(const std::string &_path)
    : stream(_path, std::ios_base::in)
  {
  }

  public: std::ifstream stream;
};

CSVFile::Cursor::Cursor(const CSVFile &_file)
  : dataPtr(gz::utils::MakeUniqueImpl<Implementation>(_file.Path()))
{
  if (!_file.Header().empty())
  {
    this->dataPtr->stream.ignore(
        std::numeric_limits<std::streamsize>::max(), '\n');
  }
}

std::ifstream &CSVFile::Cursor::Stream()
{
  return this->dataPtr->stream;
}

CSVStreamIterator CSVFile::Cursor::begin() const
{
  return CSVStreamIterator(this->dataPtr->stream);
}

CSVStreamIterator CSVFile::Cursor::end() const
{
  return CSVStreamIterator();
}

class CSVFile::Implementation
{
  public: Implementation(const std::string &_path)
    : path(_path)
  {
  }

  public: std::string path;

  public: std::vector<std::string> header;

  public: size_t numRows{0};

  public: size_t numColumns{0};
};

CSVFile::CSVFile(const std::string &_path, bool _has_header)
  : dataPtr(gz::utils::MakeUniqueImpl<Implementation>(_path))
{
  if (isFile(_path))
  {
    throw std::runtime_error("No " + _path + " file found");
  }
  if (_has_header)
  {
    std::ifstream stream{_path};
    CSVStreamIterator iterator{stream};
    this->dataPtr->header = *iterator;
  }
}

const std::string &CSVFile::Path() const
{
  return this->dataPtr->path;
}

const std::vector<std::string> &CSVFile::Header() const
{
  return this->dataPtr->header;
}

CSVFile::Cursor CSVFile::Data() const
{
  return Cursor(*this);
}

size_t CSVFile::NumRows() const
{
  auto cursor = this->Data();
  return std::count(
    std::istreambuf_iterator<char>(cursor.Stream()),
    std::istreambuf_iterator<char>(), '\n');
}

size_t CSVFile::NumColumns() const
{
  return this->Data().begin()->size();
}
