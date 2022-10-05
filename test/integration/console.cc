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

#include <gtest/gtest.h>

#include "gz/common/Console.hh"
#include "test_config.h"

using namespace ignition;

TEST(Console_TEST, LogInitAfterConsoleOut)
{
  std::string logFilename = "uri.log";
  std::string logDir = common::joinPaths(PROJECT_BINARY_PATH, "test", "uri");
  std::string logFile = common::joinPaths(logDir, logFilename);

  common::Console::SetVerbosity(4);

  // We are not logging to a file yet.
  ignerr << "This is an error" << std::endl;

  // Initialize the log file.
  ignLogInit(logDir, logFilename);

  // Run the same console output, which should output the message to the log
  // file.
  ignerr << "This is an error" << std::endl;
  std::ifstream t(logFile);
  std::string buffer((std::istreambuf_iterator<char>(t)),
                 std::istreambuf_iterator<char>());

  EXPECT_TRUE(buffer.find("This is an error") != std::string::npos)
    << "Log file content[" << buffer << "]\n";
}
