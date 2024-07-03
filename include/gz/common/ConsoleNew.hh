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

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <gz/common/Export.hh>
#include <gz/common/Util.hh>
#include <gz/utils/ImplPtr.hh>
#include "gz/common/Filesystem.hh"

#include <spdlog/logger.h>
#include <spdlog/spdlog.h>

namespace gz::common
{

  /// \brief Gazebo Console and File logging class.
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

    /// \brief Set the log destnation filename
    public: void SetLogDestination(const std::string &_filename);

    /// \brief Access the underlying spdlog logger
    public: [[nodiscard]] spdlog::logger& Logger() const;

    /// \brief Access the underlying spdlog logger, with ownership
    public: [[nodiscard]] std::shared_ptr<spdlog::logger> LoggerPtr() const;

    /// \brief Access the global gz console logger
    public: static ConsoleNew& Root();

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
    public: LogMessage(const char * _file,
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
}  // namespace gz::common

#define gzcrit (gz::common::LogMessage( \
  __FILE__, __LINE__, spdlog::level::critical).stream())
#define gzerr gz::common::LogMessage( \
  __FILE__, __LINE__, spdlog::level::err).stream()
#define gzwarn gz::common::LogMessage( \
  __FILE__, __LINE__, spdlog::level::warn).stream()
#define gzlog gz::common::LogMessage( \
  __FILE__, __LINE__, spdlog::level::info).stream()
#define gzmsg gz::common::LogMessage( \
  __FILE__, __LINE__, spdlog::level::info).stream()
#define gzdbg gz::common::LogMessage( \
  __FILE__, __LINE__, spdlog::level::debug).stream()
#define gztrace gz::common::LogMessage( \
  __FILE__, __LINE__, spdlog::level::trace).stream()

void gzLogInit(const std::string &directory, const std::string &filename)
{
  auto &root = gz::common::ConsoleNew::Root();

  std::string logPath;
  if (!directory.empty())
  {
    logPath = directory;
  } else if(!gz::common::env(GZ_HOMEDIR, logPath)) {
    root.Logger().error("Missing HOME environment variable. No log file will be generated.");
    return;
  }

  if(!gz::common::createDirectories(logPath))
  {
    root.Logger().error("Failed to create output log directory {}", logPath.c_str());
    return;
  }

  logPath = gz::common::joinPaths(logPath, filename);
  root.Logger().info("Setting log file output destination to {}", logPath.c_str());
  gz::common::ConsoleNew::Root().SetLogDestination(logPath);
  spdlog::set_default_logger(root.LoggerPtr());
}

#endif  // GZ_COMMON_CONSOLENEW_HH_
