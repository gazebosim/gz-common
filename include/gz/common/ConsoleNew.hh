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
#include <memory>
#include <sstream>
#include <string>

#include <gz/common/Export.hh>
#include <gz/utils/SuppressWarning.hh>
#include <gz/common/Util.hh>
#include <gz/utils/ImplPtr.hh>

#include <spdlog/logger.h>

namespace gz::common
{

  /// Gazebo Console and File logging class
  /// This will configure spdlog with a sane set of defaults for logging to the console as well as a file
  class GZ_COMMON_VISIBLE ConsoleNew
  {
    public: explicit ConsoleNew(const std::string &logger_name);

    /// \brief Set the console output color mode
    public: void set_color_mode(spdlog::color_mode mode);

    /// \brief Access the underlying spdlog logger
    public: [[nodiscard]] spdlog::logger& Logger() const;

    /// \brief Access the global gz console logger
    public: static ConsoleNew& Root();

    /// \brief Implementation Pointer
    GZ_UTILS_UNIQUE_IMPL_PTR(dataPtr)
  };

  /// Helper class for providing gzlog macros
  class GZ_COMMON_VISIBLE LogMessage
  {
    /// \brief Constructor
    public: LogMessage(const char * file, int line, spdlog::level::level_enum log_level);

    /// \brief Destructor
    public: ~LogMessage();

    /// \brief Get access to the underlying stream
    public: std::ostream& stream();

    /// \brief Log level
    private: spdlog::level::level_enum severity;

    /// \brief Source file location information
    private: spdlog::source_loc source_location;

    /// \brief Underlying stream
    private: std::ostringstream ss;
  };



}  // namespace gz::common

#define gzcrit (gz::common::LogMessage(__FILE__, __LINE__, spdlog::level::critical).stream())
#define gzerr gz::common::LogMessage(__FILE__, __LINE__, spdlog::level::err).stream()
#define gzwarn gz::common::LogMessage(__FILE__, __LINE__, spdlog::level::warn).stream()
#define gzlog gz::common::LogMessage(__FILE__, __LINE__, spdlog::level::info).stream()
#define gzmsg gz::common::LogMessage(__FILE__, __LINE__, spdlog::level::info).stream()
#define gzdbg gz::common::LogMessage(__FILE__, __LINE__, spdlog::level::debug).stream()
#define gztrace gz::common::LogMessage(__FILE__, __LINE__, spdlog::level::trace).stream()

#endif  // GZ_COMMON_CONSOLENEW_HH_
