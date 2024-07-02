/*
 * Copyright (C) 2016 Open Source Robotics Foundation
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
#include <memory>
#include <string>
#include <sstream>

#include <gz/common/ConsoleNew.hh>
#include <gz/common/config.hh>
#include <gz/common/Util.hh>
#include <gz/utils/NeverDestroyed.hh>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace {
class gz_split_sink : public spdlog::sinks::sink {
 public:
    ~gz_split_sink() override = default;

    void log(const spdlog::details::log_msg &msg) override
    {
      if (msg.level == spdlog::level::warn ||
          msg.level == spdlog::level::err ||
          msg.level == spdlog::level::critical)
        stderr.log(msg);
      else
        stdout.log(msg);
    }

    void flush() override
    {
      stdout.flush();
      stderr.flush();
    }

    void set_pattern(const std::string &pattern) override
    {
      stdout.set_pattern(pattern);
      stderr.set_pattern(pattern);
    }

    void set_formatter(std::unique_ptr<spdlog::formatter> sink_formatter) override
    {
      stdout.set_formatter(sink_formatter->clone());
      stderr.set_formatter(std::move(sink_formatter));
    }

    void set_color_mode(spdlog::color_mode mode)
    {
      stdout.set_color_mode(mode);
      stderr.set_color_mode(mode);
    }

 private:
  spdlog::sinks::stdout_color_sink_mt stdout;
  spdlog::sinks::stderr_color_sink_mt stderr;
};
}  // namespace

namespace gz::common
{

LogMessage::LogMessage(const char* file, int line, spdlog::level::level_enum log_level):
  severity(log_level),
  source_location(file, line, "")
{
}

LogMessage::~LogMessage()
{
  gz::common::ConsoleNew::Root().Logger().log(source_location, severity, ss.str());
}

std::ostream& LogMessage::stream()
{
  return ss;
}

class ConsoleNew::Implementation
{
 public:
  std::shared_ptr<gz_split_sink> console_sink {nullptr};

  std::shared_ptr<spdlog::sinks::basic_file_sink_mt> file_sink {nullptr};

  std::shared_ptr<spdlog::logger> logger {nullptr};
};

ConsoleNew::ConsoleNew(const std::string &logger_name):
  dataPtr(gz::utils::MakeUniqueImpl<Implementation>())
{
  this->dataPtr->console_sink = std::make_shared<gz_split_sink>();
  this->dataPtr->file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("/tmp/filename.txt", true);
  this->dataPtr->logger = std::make_shared<spdlog::logger>(spdlog::logger(logger_name,
    {
      this->dataPtr->file_sink, this->dataPtr->console_sink
    }));

  // Configure the logger
  this->dataPtr->logger->set_level(spdlog::level::trace);
  this->dataPtr->logger->flush_on(spdlog::level::err);
  spdlog::flush_every(std::chrono::seconds(5));
  spdlog::register_logger(this->dataPtr->logger);
}

void ConsoleNew::set_color_mode(spdlog::color_mode mode)
{
  this->dataPtr->console_sink->set_color_mode(mode);
}

spdlog::logger& ConsoleNew::Logger() const
{
  return *this->dataPtr->logger;
}

ConsoleNew& ConsoleNew::Root()
{
  static gz::utils::NeverDestroyed<ConsoleNew> root{"gz"};
  return root.Access();
}
}  // namespace gz::common
