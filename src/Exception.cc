/*
 * Copyright (C) 2015 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "ignition/common/Console.hh"
#include "ignition/common/ExceptionPrivate.hh"
#include "ignition/common/Exception.hh"

using namespace ignition;
using namespace common;

//////////////////////////////////////////////////
Exception::Exception()
  : dataPtr(new ExceptionPrivate)
{
}

//////////////////////////////////////////////////
Exception::Exception(const std::string &_file, const int64_t _line,
    const std::string &_msg)
  : dataPtr(new ExceptionPrivate)
{
  this->dataPtr->file = _file;
  this->dataPtr->line = _line;
  this->dataPtr->str = _msg;
  this->Print();
}

//////////////////////////////////////////////////
Exception::Exception(const Exception &_other)
  : dataPtr(new ExceptionPrivate)
{
  *dataPtr = *_other.dataPtr;
}

//////////////////////////////////////////////////
Exception::~Exception()
{
}

//////////////////////////////////////////////////
void Exception::Print() const
{
  (ignition::common::Console::err(
      this->dataPtr->file, static_cast<unsigned int>(this->dataPtr->line)))
      << "EXCEPTION: " << *this << std::endl;
}

//////////////////////////////////////////////////
std::string Exception::ErrorFile() const
{
  return this->dataPtr->file;
}

//////////////////////////////////////////////////
std::string Exception::ErrorStr() const
{
  return this->dataPtr->str;
}

//////////////////////////////////////////////////
InternalError::InternalError(const std::string &_file, const int64_t _line,
                             const std::string &_msg) :
  Exception(_file, _line, _msg)
{
}

//////////////////////////////////////////////////
AssertionInternalError::AssertionInternalError(
    const std::string & _file,
    const int64_t _line,
    const std::string &_expr,
    const std::string &_function,
    const std::string &_msg) :
  InternalError(_file, _line,
      "IGNITION ASSERTION                   \n" +
      _msg                               + "\n" +
      "In function       : " + _function + "\n" +
      "Assert expression : " + _expr     + "\n")
{
}
