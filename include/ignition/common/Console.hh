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
#ifndef IGNITION_COMMON_CONSOLE_HH_
#define IGNITION_COMMON_CONSOLE_HH_

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <ignition/common/Util.hh>
#include <ignition/common/Export.hh>
#include <ignition/common/SuppressWarning.hh>

namespace ignition
{
  namespace common
  {
    /// \brief Output an error message, if the verbose level is >= 1
    #define ignerr (ignition::common::Console::err(__FILE__, __LINE__))

    /// \brief Output a warning message, if the verbose level is >= 2
    #define ignwarn (ignition::common::Console::warn(__FILE__, __LINE__))

    /// \brief Output a message, if the verbose level is >= 3
    #define ignmsg (ignition::common::Console::msg())

    /// \brief Output a debug message, if the verbose level is >= 4
    #define igndbg (ignition::common::Console::dbg(__FILE__, __LINE__))

    /// \brief Output a message to a log file, regardless of verbosity level
    #define ignlog (ignition::common::Console::log())

    /// \brief Initialize log file with filename given by _str.
    /// If called twice, it will close currently in use and open a new
    /// log file.
    /// \param[in] _dir Name of directory in which to store the log file. Note
    /// that _dir must be relative to your home directory.
    /// \param[in] _file Name of log file for ignlog messages.
    #define ignLogInit(_dir, _file)\
        ignition::common::Console::log.Init(_dir, _file)

    /// \brief Get the full path of the directory where the log files are stored
    /// \return Full path of the directory
    #define ignLogDirectory()\
        (ignition::common::Console::log.LogDirectory())

    /// \class FileLogger FileLogger.hh common/common.hh
    /// \brief A logger that outputs messages to a file.
    class IGNITION_COMMON_VISIBLE FileLogger : public std::ostream
    {
      /// \brief Constructor.
      /// \param[in] _filename Filename to write into. If empty,
      /// FileLogger::Init must be called separately.
      public: explicit FileLogger(const std::string &_filename = "");

      /// \brief Destructor.
      public: virtual ~FileLogger();

      /// \brief Initialize the file logger.
      /// \param[in] _directory Name of directory that holds the log file.
      /// \param[in] _filename Name of the log file to write output into.
      public: void Init(const std::string &_directory,
                        const std::string &_filename);

      /// \brief Output a filename and line number, then return a reference
      /// to the logger.
      /// \return Reference to this logger.
      public: virtual FileLogger &operator()();

      /// \brief Output a filename and line number, then return a reference
      /// to the logger.
      /// \param[in] _file Filename to output.
      /// \param[in] _line Line number in the _file.
      /// \return Reference to this logger.
      public: virtual FileLogger &operator()(
                  const std::string &_file, int _line);

      /// \brief Get the full path of the directory where all the log files
      /// are stored.
      /// \return Full path of the directory.
      public: std::string LogDirectory() const;

      /// \brief String buffer for the file logger.
      protected: class Buffer : public std::stringbuf
                 {
                   /// \brief Constructor.
                   /// \param[in] _filename Filename to write into.
                   public: explicit Buffer(const std::string &_filename);

                   /// \brief Destructor.
                   public: virtual ~Buffer();

                   /// \brief Sync the stream (output the string buffer
                   /// contents).
                   /// \return Return 0 on success.
                   public: virtual int sync();

                   /// \brief Stream to output information into.
                   public: std::ofstream *stream;
                 };

      IGN_COMMON_WARN_IGNORE__DLL_INTERFACE_MISSING
      /// \brief Stores the full path of the directory where all the log files
      /// are stored.
      private: std::string logDirectory;
      IGN_COMMON_WARN_RESUME__DLL_INTERFACE_MISSING

      /// \brief True if initialized.
      private: bool initialized;
    };

    /// \class Logger Logger.hh common/common.hh
    /// \brief Terminal logger.
    class IGNITION_COMMON_VISIBLE Logger : public std::ostream
    {
      /// \enum LogType.
      /// \brief Output destination type.
      public: enum LogType
              {
                /// \brief Output to stdout.
                STDOUT,
                /// \brief Output to stderr.
                STDERR
              };

      /// \brief Constructor.
      /// \param[in] _prefix String to use as prefix when logging to file.
      /// \param[in] _color Color of the output stream.
      /// \param[in] _type Output destination type (STDOUT, or STDERR)
      /// \param[in] _verbosity Verbosity level.
      public: Logger(const std::string &_prefix, const int _color,
                     const LogType _type, const int _verbosity);

      /// \brief Destructor.
      public: virtual ~Logger();

      /// \brief Access operator.
      /// \return Reference to this logger.
      public: virtual Logger &operator()();

      /// \brief Output a filename and line number, then return a reference
      /// to the logger.
      /// \param[in] _file Filename to output.
      /// \param[in] _line Line number in the _file.
      /// \return Reference to this logger.
      public: virtual Logger &operator()(
                  const std::string &_file, int _line);

      /// \brief String buffer for the base logger.
      protected: class Buffer : public std::stringbuf
                 {
                   /// \brief Constructor.
                   /// \param[in] _type Output destination type
                   /// (STDOUT, or STDERR)
                   /// \param[in] _color Color of the output stream.
                   /// \param[in] _verbosity Verbosity level.
                   public: Buffer(LogType _type, const int _color,
                                  const int _verbosity);

                   /// \brief Destructor.
                   public: virtual ~Buffer();

                   /// \brief Sync the stream (output the string buffer
                   /// contents).
                   /// \return Return 0 on success.
                   public: virtual int sync();

                   /// \brief Destination type for the messages.
                   public: LogType type;

                   /// \brief ANSI color code using Select Graphic Rendition
                   /// parameters (SGR). See
                   /// http://en.wikipedia.org/wiki/ANSI_escape_code#Colors
                   public: int color;

                   /// \brief Level of verbosity
                   public: int verbosity;
                 };

      IGN_COMMON_WARN_IGNORE__DLL_INTERFACE_MISSING
      /// \brief Prefix to use when logging to file.
      private: std::string prefix;
      IGN_COMMON_WARN_RESUME__DLL_INTERFACE_MISSING
    };

    /// \class Console Console.hh common/common.hh
    /// \brief Container for loggers, and global logging options
    /// (such as verbose vs. quiet output).
    class IGNITION_COMMON_VISIBLE Console
    {
      /// \brief Set verbosity, where
      /// <= 0: No output,
      /// 1: Error messages,
      /// 2: Error and warning messages,
      /// 3: Error, warning, and info messages,
      /// >= 4: Error, warning, info, and debug messages.
      /// \param[in] _level The new verbose level.
      public: static void SetVerbosity(const int _level);

      /// \brief Get the verbose level.
      /// \return The level of verbosity.
      /// \sa SetVerbosity(const int _level)
      public: static int Verbosity();

      /// \brief Add a custom prefix in front of the default prefixes.
      ///
      /// By default, the custom prefix is an empty string, so the messages
      /// start as:
      ///
      /// [Err], [Wrn], [Msg], [Dbg]
      ///
      /// If you set the prefix to "-my-", for example, they become:
      ///
      /// -my-[Err], -my-[Wrn], -my-[Msg], -my-[Dbg]
      ///
      /// \param[in] _customPrefix Prefix string.
      /// \sa std::string Prefix() const
      public: static void SetPrefix(const std::string &_customPrefix);

      /// \brief Get custom prefix. This is empty by default.
      /// \return The custom prefix.
      /// \sa void SetPrefix(const std::string &_customPrefix)
      public: static std::string Prefix();

      /// \brief Global instance of the message logger.
      public: static Logger msg;

      /// \brief Global instance of the error logger.
      public: static Logger err;

      /// \brief Global instance of the debug logger.
      public: static Logger dbg;

      /// \brief Global instance of the warning logger.
      public: static Logger warn;

      /// \brief Global instance of the file logger.
      public: static FileLogger log;

      /// \brief The level of verbosity, the default level is 1.
      private: static int verbosity;
    };
  }
}
#endif
