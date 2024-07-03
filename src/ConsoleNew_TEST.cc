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

#include "gz/common/ConsoleNew.hh"
#include "gz/common/TempDirectory.hh"
#include "gz/common/Util.hh"

#include <spdlog/spdlog.h>

class ConsoleNew_TEST : public ::testing::Test {
  protected: virtual void SetUp()
  {
    this->temp = std::make_unique<gz::common::TempDirectory>(
        "test", "gz_common", true);
    ASSERT_TRUE(this->temp->Valid());
    gz::common::setenv(GZ_HOMEDIR, this->temp->Path());
  }

  /// \brief Clear out all the directories we produced during this test.
  public: virtual void TearDown()
  {
    EXPECT_TRUE(gz::common::unsetenv(GZ_HOMEDIR));
  }

  /// \brief Temporary directory to run test in
  private: std::unique_ptr<gz::common::TempDirectory> temp;
};

TEST_F(ConsoleNew_TEST, NonRootLogger)
{
  gz::common::ConsoleNew test_console("test");
  auto test_logger = test_console.Logger();
  test_logger.log(spdlog::level::trace, "This is a trace message");
  test_logger.log(spdlog::level::debug , "This is a debug message");
  test_logger.log(spdlog::level::info, "This is an info message");
  test_logger.log(spdlog::level::warn, "This is a warn message");
  test_logger.log(spdlog::level::err, "This is an error message");
  test_logger.log(spdlog::level::critical, "This is a critical error message");
}

TEST_F(ConsoleNew_TEST, RootLogger)
{
  auto gz_logger = gz::common::ConsoleNew::Root().Logger();
  gz_logger.log(spdlog::level::trace, "This is a trace message");
  gz_logger.log(spdlog::level::debug , "This is a debug message");
  gz_logger.log(spdlog::level::info, "This is an info message");
  gz_logger.log(spdlog::level::warn, "This is a warn message");
  gz_logger.log(spdlog::level::err, "This is an error message");
  gz_logger.log(spdlog::level::critical, "This is a critical error message");
}

TEST_F(ConsoleNew_TEST, RootLoggerColor)
{
  gz::common::ConsoleNew::Root().SetColorMode(spdlog::color_mode::always);
  spdlog::get("gz")->log(spdlog::level::trace, "This is a trace message");
  spdlog::get("gz")->log(spdlog::level::debug , "This is a debug message");
  spdlog::get("gz")->log(spdlog::level::info, "This is an info message");
  spdlog::get("gz")->log(spdlog::level::warn, "This is a warn message");
  spdlog::get("gz")->log(spdlog::level::err, "This is an error message");
  spdlog::get("gz")->log(spdlog::level::critical, "This is a critical error message");
}

TEST_F(ConsoleNew_TEST, RootLoggerNoColor)
{
  gz::common::ConsoleNew::Root().SetColorMode(spdlog::color_mode::never);
  spdlog::get("gz")->trace("This is a trace message");
  spdlog::get("gz")->debug("This is a debug message");
  spdlog::get("gz")->info("This is an info message");
  spdlog::get("gz")->warn("This is a warning message");
  spdlog::get("gz")->error("This is an error message");
  spdlog::get("gz")->critical("This is a critical message");
}

TEST_F(ConsoleNew_TEST, RootLoggerMacros)
{
    gztrace << "This is a trace message";
    gzdbg << "This is a debug message";
    gzmsg << "This is an info message";
    gzwarn << "This is a warning message";
    gzerr << "This is an error message";
    gzcrit << "This is a critical message";
}
