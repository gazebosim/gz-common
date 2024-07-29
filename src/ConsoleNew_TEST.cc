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

#include <gtest/gtest.h>

#include <memory>
#include <string>

#include <gz/common/ConsoleNew.hh>
#include <gz/common/TempDirectory.hh>
#include <gz/common/Util.hh>

#include <spdlog/spdlog.h>

/// \brief Test ConsoleNew.
class ConsoleNew_TEST : public ::testing::Test
{
  // Documentation inherited
  protected: void SetUp() override
  {
    this->temp = std::make_unique<gz::common::TempDirectory>(
      "test", "gz_common", false);
    ASSERT_TRUE(this->temp->Valid());
    EXPECT_TRUE(gz::common::setenv(GZ_HOMEDIR, this->temp->Path()));
  }

  /// \brief Clear out all the directories we produced during this test.
  public: void TearDown() override
  {
    gzLogClose();
    EXPECT_TRUE(gz::common::unsetenv(GZ_HOMEDIR));
  }

  /// \brief Temporary directory to run test in.
  private: std::unique_ptr<gz::common::TempDirectory> temp;
};

/// \brief Read the log content.
/// \param[in] _filename Log filename.
/// \return Log content.
std::string GetLogContent(const std::string &_filename)
{
  // Get the absolute path
  std::string path;
  EXPECT_TRUE(gz::common::env(GZ_HOMEDIR, path));
  path = gz::common::joinPaths(path, _filename);
  EXPECT_TRUE(gz::common::exists(path));

  // Open the log file, and read back the string
  std::ifstream ifs(path.c_str(), std::ios::in);
  std::string loggedString;

  while (!ifs.eof())
  {
    std::string line;
    std::getline(ifs, line);
    loggedString += line;
  }

  return loggedString;
}

/////////////////////////////////////////////////
TEST_F(ConsoleNew_TEST, NonRootLogger)
{
  // Create a unique directory path
  auto path = gz::common::uuid();

  // Initialize logging
  gzLogInit("", "test.log");

  gz::common::ConsoleNew testConsole("test");
  auto testLogger = testConsole.Logger();

  testing::internal::CaptureStdout();
  testing::internal::CaptureStderr();

  testLogger.log(spdlog::level::trace,    "This is a trace message");
  testLogger.log(spdlog::level::debug ,   "This is a debug message");
  testLogger.log(spdlog::level::info,     "This is an info message");
  testLogger.log(spdlog::level::warn,     "This is a warn message");
  testLogger.log(spdlog::level::err,      "This is an error message");
  testLogger.log(spdlog::level::critical, "This is a critical error message");

  std::string stdOut = testing::internal::GetCapturedStdout();
  std::string stdErr = testing::internal::GetCapturedStderr();

  for (auto word : {"info"})
    EXPECT_TRUE(stdOut.find(word) != std::string::npos);

  for (auto word : {"warn", "error", "critical"})
    EXPECT_TRUE(stdErr.find(word) != std::string::npos);
}

/////////////////////////////////////////////////
TEST_F(ConsoleNew_TEST, RootLogger)
{
  auto gzLogger = gz::common::ConsoleNew::Root().Logger();

  testing::internal::CaptureStdout();
  testing::internal::CaptureStderr();

  gzLogger.log(spdlog::level::trace,    "This is a trace message");
  gzLogger.log(spdlog::level::debug ,   "This is a debug message");
  gzLogger.log(spdlog::level::info,     "This is an info message");
  gzLogger.log(spdlog::level::warn,     "This is a warn message");
  gzLogger.log(spdlog::level::err,      "This is an error message");
  gzLogger.log(spdlog::level::critical, "This is a critical error message");

  std::string stdOut = testing::internal::GetCapturedStdout();
  std::string stdErr = testing::internal::GetCapturedStderr();

  for (auto word : {"info"})
    EXPECT_TRUE(stdOut.find(word) != std::string::npos);

  for (auto word : {"warn", "error", "critical"})
    EXPECT_TRUE(stdErr.find(word) != std::string::npos);
}

/////////////////////////////////////////////////
TEST_F(ConsoleNew_TEST, RootLoggerColor)
{
  gz::common::ConsoleNew::Root().SetColorMode(spdlog::color_mode::always);
  auto logger = spdlog::get("gz");
  ASSERT_TRUE(logger);

  testing::internal::CaptureStdout();
  testing::internal::CaptureStderr();

  logger->log(spdlog::level::trace,    "This is a trace message");
  logger->log(spdlog::level::debug ,   "This is a debug message");
  logger->log(spdlog::level::info,     "This is an info message");
  logger->log(spdlog::level::warn,     "This is a warn message");
  logger->log(spdlog::level::err,      "This is an error message");
  logger->log(spdlog::level::critical, "This is a critical error message");

  std::string stdOut = testing::internal::GetCapturedStdout();
  std::string stdErr = testing::internal::GetCapturedStderr();

  for (auto word : {"info"})
    EXPECT_TRUE(stdOut.find(word) != std::string::npos);

  for (auto word : {"warn", "error", "critical"})
    EXPECT_TRUE(stdErr.find(word) != std::string::npos);
}

/////////////////////////////////////////////////
TEST_F(ConsoleNew_TEST, RootLoggerNoColor)
{
  gz::common::ConsoleNew::Root().SetColorMode(spdlog::color_mode::never);
  auto logger = spdlog::get("gz");
  ASSERT_TRUE(logger);

  testing::internal::CaptureStdout();
  testing::internal::CaptureStderr();

  logger->trace("This is a trace message");
  logger->debug("This is a debug message");
  logger->info("This is an info message");
  logger->warn("This is a warning message");
  logger->error("This is an error message");
  logger->critical("This is a critical message");

  std::string stdOut = testing::internal::GetCapturedStdout();
  std::string stdErr = testing::internal::GetCapturedStderr();

  for (auto word : {"info"})
    EXPECT_TRUE(stdOut.find(word) != std::string::npos);

  for (auto word : {"warn", "error", "critical"})
    EXPECT_TRUE(stdErr.find(word) != std::string::npos);
}

/////////////////////////////////////////////////
TEST_F(ConsoleNew_TEST, RootLoggerMacros)
{
  testing::internal::CaptureStdout();
  testing::internal::CaptureStderr();

  gztrace << "This is a trace message";
  gzdbg   << "This is a debug message";
  gzmsg   << "This is an info message";
  gzwarn  << "This is a warning message";
  gzerr   << "This is an error message";
  gzcrit  << "This is a critical message";

  std::string stdOut = testing::internal::GetCapturedStdout();
  std::string stdErr = testing::internal::GetCapturedStderr();

  for (auto word : {"info"})
    EXPECT_TRUE(stdOut.find(word) != std::string::npos);

  for (auto word : {"warn", "error", "critical"})
    EXPECT_TRUE(stdErr.find(word) != std::string::npos);
}

/////////////////////////////////////////////////
TEST_F(ConsoleNew_TEST, LogToFile)
{
  gzLogInit("", "test.log");

  gztrace << "This is a trace message";
  gzdbg   << "This is a debug message";
  gzmsg   << "This is an info message";
  gzwarn  << "This is a warning message";
  gzerr   << "This is an error message";
  gzcrit  << "This is a critical message";

  // gzLogInit installs a global handler.
  spdlog::trace("This is a trace message");
  spdlog::debug("This is a debug message");
  spdlog::info("This is an info message");
  spdlog::warn("This is a warning message");
  spdlog::error("This is an error message");
  spdlog::critical("This is a critical message");

  std::string logContent = GetLogContent("test.log");
  for (auto word : {"info", "warn", "error", "critical"})
    EXPECT_TRUE(logContent.find(word) != std::string::npos);

  auto dir = gzLogDirectory();
}
