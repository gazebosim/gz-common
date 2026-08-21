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

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <vector>

#include <gz/common/testing/RedirectConsoleStream.hh>
#include <gz/common/Console.hh>
#include <gz/common/testing/TestPaths.hh>
#include <gz/utils/SuppressWarning.hh>

#include <spdlog/sinks/base_sink.h>

namespace gz::common::testing
{

/// \brief A spdlog sink that redirects logs to an ostream with level filtering.
///
/// Level filtering is necessary to maintain the expected separation between
/// standard output (stdout) and standard error (stderr). Gazebo's logging
/// system typically routes Trace, Debug, and Info levels to stdout, and
/// Warn, Error, and Critical levels to stderr.
///
/// Since spdlog's built-in sinks only support a minimum level threshold
/// (via set_level), this custom sink implements a range-based filter
/// (min and max levels) to ensure that RedirectStdout only captures
/// stdout-bound logs and RedirectStderr only captures stderr-bound logs.
class RedirectConsoleSink : public spdlog::sinks::base_sink<std::mutex>
{
  /// \brief Constructor.
  /// \param[in] _os Output stream to write to.
  /// \param[in] _min Minimum log level to capture.
  /// \param[in] _max Maximum log level to capture.
  public: RedirectConsoleSink(std::ostream &_os,
                              spdlog::level::level_enum _min,
                              spdlog::level::level_enum _max)
    : os(_os), minLevel(_min), maxLevel(_max)
  {
  }

  /// \brief Logic for processing a log message.
  /// \param[in] _msg Log message to process.
  protected: void sink_it_(const spdlog::details::log_msg &_msg) override
  {
    if (_msg.level >= this->minLevel && _msg.level <= this->maxLevel)
    {
      spdlog::memory_buf_t formatted;
      this->formatter_->format(_msg, formatted);
      this->os.write(formatted.data(),
                     static_cast<std::streamsize>(formatted.size()));
    }
  }

  /// \brief Logic for flushing the output stream.
  protected: void flush_() override
  {
    this->os.flush();
  }

  /// \brief Output stream where logs are written.
  private: std::ostream &os;

  /// \brief Minimum log level to capture.
  private: spdlog::level::level_enum minLevel;

  /// \brief Maximum log level to capture.
  private: spdlog::level::level_enum maxLevel;
};

class RedirectConsoleStream::Implementation
{
  /// \brief Console source being redirected
  public: StreamSource source;

  /// \brief Filename to write console output to
  public: std::string sinkPath {""};

  /// \brief File stream for the redirected output
  public: std::ofstream fileStream;

  /// \brief Original stream buffer to restore
  public: std::streambuf *oldBuf {nullptr};

  /// \brief C++ stream being redirected (cout or cerr)
  public: std::ostream *stream {nullptr};

  /// \brief Custom spdlog sink for capturing Gazebo logs
  public: std::shared_ptr<RedirectConsoleSink> spdlogSink;

  /// \brief Remove any console redirection, restoring original sink
  public: void RemoveRedirection();

  /// \brief Destructor
  public: ~Implementation();
};

//////////////////////////////////////////////////
void RedirectConsoleStream::Implementation::RemoveRedirection()
{
  if (this->stream && this->oldBuf)
  {
    this->stream->rdbuf(this->oldBuf);
    this->oldBuf = nullptr;
    this->stream = nullptr;
  }

  if (this->spdlogSink)
  {
    auto &sinks = gz::common::Console::Root().RawLogger().sinks();
    sinks.erase(std::remove(sinks.begin(), sinks.end(), this->spdlogSink),
                sinks.end());
    this->spdlogSink.reset();
  }

  if (this->fileStream.is_open())
  {
    this->fileStream.close();
  }
}

//////////////////////////////////////////////////
RedirectConsoleStream::Implementation::~Implementation()
{
  this->RemoveRedirection();

  if (!this->sinkPath.empty() && common::exists(this->sinkPath))
  {
    common::removeFile(this->sinkPath);
  }
}

//////////////////////////////////////////////////
RedirectConsoleStream::RedirectConsoleStream(const StreamSource &_source,
    const std::string &_destination):
  dataPtr(gz::utils::MakeUniqueImpl<Implementation>())
{
  this->dataPtr->sinkPath = _destination;

  if (this->dataPtr->sinkPath.empty())
  {
    gzerr << "Failed to open sink file: console redirection disabled "
      << "(empty filename)" << std::endl;
    return;
  }

  if (common::exists(this->dataPtr->sinkPath))
  {
    gzerr << "Failed to open sink file: console redirection disabled "
      << "(destination exists)" << std::endl;
    return;
  }

  this->dataPtr->fileStream.open(this->dataPtr->sinkPath);
  if (!this->dataPtr->fileStream.is_open())
  {
    gzerr << "Failed to open sink file: console redirection disabled "
      << "(could not open file)" << std::endl;
    return;
  }

  spdlog::level::level_enum minLevel;
  spdlog::level::level_enum maxLevel;

  if (_source == StreamSource::STDOUT)
  {
    this->dataPtr->stream = &std::cout;
    minLevel = spdlog::level::trace;
    maxLevel = spdlog::level::info;
  }
  else
  {
    this->dataPtr->stream = &std::cerr;
    minLevel = spdlog::level::warn;
    maxLevel = spdlog::level::critical;
  }

  // Redirect C++ stream
  this->dataPtr->oldBuf = this->dataPtr->stream->rdbuf(
      this->dataPtr->fileStream.rdbuf());

  // Add spdlog sink
  this->dataPtr->spdlogSink = std::make_shared<RedirectConsoleSink>(
      *this->dataPtr->stream, minLevel, maxLevel);

  auto &logger = gz::common::Console::Root().RawLogger();

  logger.sinks().push_back(this->dataPtr->spdlogSink);
}

//////////////////////////////////////////////////
std::string RedirectConsoleStream::GetString()
{
  this->dataPtr->RemoveRedirection();

  // Read the file contents and return to the user
  std::ifstream input(this->dataPtr->sinkPath);
  std::stringstream buffer;
  buffer << input.rdbuf();
  return buffer.str();
}

//////////////////////////////////////////////////
bool RedirectConsoleStream::Active() const
{
  return !this->dataPtr->sinkPath.empty() && this->dataPtr->oldBuf != nullptr;
}

}  // namespace gz::common::testing
