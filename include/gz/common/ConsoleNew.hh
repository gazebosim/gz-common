/*
 * Copyright (C) 2024 Open Source Robotics Foundation
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
#ifndef GZ_COMMON_CONSOLENEW_HH_
#define GZ_COMMON_CONSOLENEW_HH_

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include <gz/common/Export.hh>
#include <gz/common/Filesystem.hh>
#include <gz/common/Util.hh>
#include <gz/utils/ImplPtr.hh>
#include <gz/utils/SuppressWarning.hh>

#include <spdlog/logger.h>
#include <spdlog/spdlog.h>

namespace gz::common
{
  /// \brief Gazebo console and file logging class.
  /// This will configure spdlog with a sane set of defaults for logging to the
  /// console as well as a file.
  class GZ_COMMON_VISIBLE ConsoleNew
  {
    /// \brief Class constructor.
    /// \param[in] _loggerName Logger name.
    public: explicit ConsoleNew(const std::string &_loggerName);

    /// \brief Set the console output color mode.
    ///\param[in] _mode Color mode.  
    public: void SetColorMode(spdlog::color_mode _mode);

    /// \brief Set the log destination filename.
    /// \param[in] _filename Log file name.
    public: void SetLogDestination(const std::string &_filename);

    /// \brief Get the log destination filename.
    /// \return Log file name.
    public: std::string LogDestination() const;

    /// \brief Access the underlying spdlog logger.
    /// \return The spdlog logger.
    public: [[nodiscard]] spdlog::logger &Logger() const;

    /// \brief Access the underlying spdlog logger, with ownership.
    /// \return The spdlog logger.
    public: [[nodiscard]] std::shared_ptr<spdlog::logger> LoggerPtr() const;

    /// \brief Access the global gz console logger.
    /// \return The gz consoler logger.
    public: static ConsoleNew &Root();

    /// \brief Implementation Pointer.
    GZ_UTILS_UNIQUE_IMPL_PTR(dataPtr)
  };

  /// Helper class for providing gzlog macros.
  class GZ_COMMON_VISIBLE LogMessage
  {
    /// \brief Constructor.
    /// \param[in] _file Filename.
    /// \param[in] _line Line number.
    /// \param[in] _logLevel Log level.
    public: LogMessage(const char *_file,
                       int _line,
                       spdlog::level::level_enum _logLevel);

    /// \brief Destructor.
    public: ~LogMessage();

    /// \brief Get access to the underlying stream.
    /// \return The underlying stream.
    public: std::ostream &stream();

    /// \brief Log level.
    private: spdlog::level::level_enum severity;

    /// \brief Source file location information.
    private: spdlog::source_loc sourceLocation;

    /// \brief Underlying stream.
    private: std::ostringstream ss;
  };

  /// \brief Helper class for providing global options.
  class GZ_COMMON_VISIBLE ConsoleGlobal
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

    /// \brief ToDo.
    public: friend class LogMessage;

    /// \brief The level of verbosity, the default level is 1.
    private: static int verbosity;

    //GZ_UTILS_WARN_IGNORE__DLL_INTERFACE_MISSING
    /// \brief A custom prefix. See SetPrefix().
    private: static std::string customPrefix;
    //GZ_UTILS_WARN_RESUME__DLL_INTERFACE_MISSING
  };
}  // namespace gz::common

/// \brief Output a critical message.
#define gzcrit (gz::common::LogMessage( \
  __FILE__, __LINE__, spdlog::level::critical).stream())

/// \brief Output an error message.
#define gzerr gz::common::LogMessage( \
  __FILE__, __LINE__, spdlog::level::err).stream()

/// \brief Output a warning message.
#define gzwarn gz::common::LogMessage( \
  __FILE__, __LINE__, spdlog::level::warn).stream()

/// \brief Output a message to a log file.
#define gzlog gz::common::LogMessage( \
  __FILE__, __LINE__, spdlog::level::info).stream()

/// \brief Output a message.
#define gzmsg gz::common::LogMessage( \
  __FILE__, __LINE__, spdlog::level::info).stream()

/// \brief Output a debug message.
#define gzdbg gz::common::LogMessage( \
  __FILE__, __LINE__, spdlog::level::debug).stream()

/// \brief Output a trace message.
#define gztrace gz::common::LogMessage( \
  __FILE__, __LINE__, spdlog::level::trace).stream()

/// \brief Initialize the Gazebo log.
/// \param[in] _directory Directory to log.
/// \param[in] _filename Filename to log.
void gzLogInit(const std::string &_directory, const std::string &_filename)
{
  auto &root = gz::common::ConsoleNew::Root();

  std::string logPath;
  if (!_directory.empty())
  {
    logPath = _directory;
  } else if (!gz::common::env(GZ_HOMEDIR, logPath))
  {
    root.Logger().error(
      "Missing HOME environment variable. No log file will be generated.");
    return;
  }

  if (!gz::common::createDirectories(logPath))
  {
    root.Logger().error("Failed to create output log directory {}",
      logPath.c_str());
    return;
  }

  logPath = gz::common::joinPaths(logPath, _filename);
  root.Logger().info("Setting log file output destination to {}",
    logPath.c_str());
  gz::common::ConsoleNew::Root().SetLogDestination(logPath);
  spdlog::set_default_logger(root.LoggerPtr());
}

/// \brief Close the file used for logging.
void gzLogClose()
{
  auto filePath = gz::common::ConsoleNew::Root().LogDestination();
  std::ifstream ifs(filePath);
  if (!ifs.is_open())
    std::filesystem::remove(filePath);
}

/// \brief Get the full path of the directory where the log files are stored
/// \return Full path of the directory.
std::string gzLogDirectory()
{
  std::filesystem::path path = gz::common::ConsoleNew::Root().LogDestination();
  return path.parent_path();
}

#endif  // GZ_COMMON_CONSOLENEW_HH_
