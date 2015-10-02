/*
 * Copyright (C) 2015 Open Source Robotics Foundation
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

#ifndef _IGNITION_COMMON_EXCEPTION_HH_
#define _IGNITION_COMMON_EXCEPTION_HH_

#include <cstdint>
#include <memory>
#include <string>
#include <iostream>
#include <sstream>

#include "ignition/common/Util.hh"

namespace ignition
{
  namespace common
  {
    /// \brief This macro logs an error to the throw stream and throws
    /// an exception that contains the file name and line number.
    #define ignthrow(msg) {std::ostringstream throwStream;\
      throwStream << msg << std::endl << std::flush;\
      throw ignition::common::Exception(__FILE__, __LINE__, \
      throwStream.str()); }

    class ExceptionPrivate;

    /// \class Exception Exception.hh common/Exception.hh
    /// \brief Class for generating exceptions
    class IGNITION_VISIBLE Exception
    {
      /// \brief Constructor
      public: Exception();

      /// \brief Copy constructor
      public: Exception(const Exception &_other);

      /// \brief Default constructor
      /// \param[in] _file File name
      /// \param[in] _line Line number where the error occurred
      /// \param[in] _msg Error message
      public: Exception(const std::string &_file,
                        const int64_t _line,
                        const std::string &_msg);

      /// \brief Destructor
      public: virtual ~Exception();

      /// \brief Return the error function
      /// \return The error function name
      public: std::string ErrorFile() const;

      /// \brief Return the error string
      /// \return The error string
      public: std::string ErrorStr() const;

      /// \brief Print the exception to std out.
      public: void Print() const;

      /// \brief stream insertion operator for Ignition Error
      /// \param[in] _out the output stream
      /// \param[in] _err the exception
      public: friend std::ostream &operator<<(std::ostream &_out,
                  const ignition::common::Exception &_err)
              {
                return _out << _err.ErrorStr();
              }

      /// \brief Private data pointer.
      private: std::unique_ptr<ExceptionPrivate> dataPtr;
    };

    /// \class InternalError Exception.hh common/Exception.hh
    /// \brief Class for generating Internal Ignition Errors:
    ///        those errors which should never happen and
    ///        represent programming bugs.
    class IGNITION_VISIBLE InternalError : public Exception
    {
      /// \brief Constructor
      public: InternalError() = default;

      /// \brief Default constructor
      /// \param[in] _file File name
      /// \param[in] _line Line number where the error occurred
      /// \param[in] _msg Error message
      public: InternalError(const std::string &_file,
                            const int64_t _line,
                            const std::string &_msg);

      /// \brief Destructor
      public: virtual ~InternalError() = default;
    };

    /// \class AssertionInternalError Exception.hh common/Exception.hh
    /// \brief Class for generating Exceptions which come from
    ///        ignition assertions. They include information about the
    ///        assertion expression violated, function where problem
    ///        appeared and assertion debug message.
    class IGNITION_VISIBLE AssertionInternalError : public InternalError
    {
      /// \brief Constructor for assertions
      /// \param[in] _file File name
      /// \param[in] _line Line number where the error occurred
      /// \param[in] _expr Assertion expression failed resulting in an
      ///                  internal error
      /// \param[in] _function Function where assertion failed
      /// \param[in] _msg Error message
      public: AssertionInternalError(const std::string &_file,
                                     const int64_t _line,
                                     const std::string &_expr,
                                     const std::string &_function,
                                     const std::string &_msg = "");
      /// \brief Destructor
      public: virtual ~AssertionInternalError() = default;
    };
  }
}

#endif
