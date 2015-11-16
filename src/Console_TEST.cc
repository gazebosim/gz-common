/*
 * Copyright (C) 2012-2014 Open Source Robotics Foundation
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
#include <stdlib.h>
#include <boost/filesystem.hpp>

#include "ignition/common/Time.hh"
#include "ignition/common/Console.hh"

const int g_messageRepeat = 4;

class Console_TEST {};

std::string GetLogContent(const std::string &_filename)
{
  // Get the absolute path
  boost::filesystem::path path = getenv("HOME");
  path /= _filename;

  // Open the log file, and read back the string
  std::ifstream ifs(path.string().c_str(), std::ios::in);
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
/// \brief Test Console::Init and Console::Log
TEST(Console_TEST, NoInitAndLog)
{
  std::cout << "(" << ignition::common::SystemTimeISO() << ")\n";

  // Log the string
  std::string logString = "this is a test";
  ignlog << logString << std::endl;

  // Get the absolute log file path
  std::string logPath = ".ignition/auto_default.log";

  // Expect to find the string in the log file
  EXPECT_TRUE(GetLogContent(logPath).find(logString) != std::string::npos);

  // Cleanup
  boost::filesystem::path path = getenv("HOME");
  path /= logPath;
  boost::filesystem::remove_all(path);
}

/////////////////////////////////////////////////
/// \brief Test Console::Init and Console::Log
TEST(Console_TEST, InitAndLog)
{
  // Create a unique directory path
  boost::filesystem::path path = boost::filesystem::unique_path();

  // Initialize logging
  ignLogInit(path.string(), "test.log");

  // Log the string
  std::string logString = "this is a test";
  ignlog << logString << std::endl;

  // Get the absolute path
  boost::filesystem::path basePath = getenv("HOME");
  basePath /= path;

  // Get the absolute log file path
  boost::filesystem::path logPath = path / "test.log";

  // Expect to find the string in the log file
  EXPECT_TRUE(GetLogContent(logPath.string()).find(logString) !=
              std::string::npos);

  // Cleanup
  boost::filesystem::remove_all(basePath);
}

//////////////////////////////////////////////////
/// \brief Test Console::Log with \n characters
TEST(Console_TEST, LogSlashN)
{
  // Create a unique directory path
  boost::filesystem::path path = boost::filesystem::unique_path();

  // Initialize logging
  ignLogInit(path.string(), "test.log");

  // Get the absolute log file path
  boost::filesystem::path logPath = path / "test.log";

  std::string logString = "this is a log test";

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    ignlog << logString << " _n__ " << i << '\n';
  }

  std::string logContent = GetLogContent(logPath.string());

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    std::ostringstream stream;
    stream << logString << " _n__ " << i;
    EXPECT_TRUE(logContent.find(stream.str()) != std::string::npos);
  }
}

//////////////////////////////////////////////////
/// \brief Test Console::Log with std::endl
TEST(Console_TEST, LogStdEndl)
{
  // Create a unique directory path
  boost::filesystem::path path = boost::filesystem::unique_path();

  // Initialize logging
  ignLogInit(path.string(), "test.log");

  // Get the absolute log file path
  boost::filesystem::path logPath = path / "test.log";

  std::string logString = "this is a log test";

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    ignlog << logString << " endl " << i << std::endl;
  }

  std::string logContent = GetLogContent(logPath.string());

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    std::ostringstream stream;
    stream << logString << " endl " << i;
    EXPECT_TRUE(logContent.find(stream.str()) != std::string::npos);
  }
}

//////////////////////////////////////////////////
/// \brief Test Console::ColorWarn with \n characters
TEST(Console_TEST, ColorWarnSlashN)
{
  // Create a unique directory path
  boost::filesystem::path path = boost::filesystem::unique_path();

  // Initialize logging
  ignLogInit(path.string(), "test.log");

  // Get the absolute log file path
  boost::filesystem::path logPath = path / "test.log";

  std::string logString = "this is a warning test";

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    ignwarn << logString << " _n__ " << i << '\n';
  }

  std::string logContent = GetLogContent(logPath.string());

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    std::ostringstream stream;
    stream << logString << " _n__ " << i;
    EXPECT_TRUE(logContent.find(stream.str()) != std::string::npos);
  }
}

//////////////////////////////////////////////////
/// \brief Test Console::ColorWarn with std::endl
TEST(Console_TEST, ColorWarnStdEndl)
{
  // Create a unique directory path
  boost::filesystem::path path = boost::filesystem::unique_path();

  // Initialize logging
  ignLogInit(path.string(), "test.log");

  // Get the absolute log file path
  boost::filesystem::path logPath = path / "test.log";

  std::string logString = "this is a warning test";

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    ignwarn << logString << " endl " << i << std::endl;
  }

  std::string logContent = GetLogContent(logPath.string());

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    std::ostringstream stream;
    stream << logString << " endl " << i;
    EXPECT_TRUE(logContent.find(stream.str()) != std::string::npos);
  }
}

//////////////////////////////////////////////////
/// \brief Test Console::ColorDbg with \n characters
TEST(Console_TEST, ColorDbgSlashN)
{
  // Create a unique directory path
  boost::filesystem::path path = boost::filesystem::unique_path();

  // Initialize logging
  ignLogInit(path.string(), "test.log");

  // Get the absolute log file path
  boost::filesystem::path logPath = path / "test.log";

  std::string logString = "this is a dbg test";

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    igndbg << logString << " _n__ " << i << '\n';
  }

  std::string logContent = GetLogContent(logPath.string());

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    std::ostringstream stream;
    stream << logString << " _n__ " << i;
    EXPECT_TRUE(logContent.find(stream.str()) != std::string::npos);
  }
}

//////////////////////////////////////////////////
/// \brief Test Console::ColorDbg with std::endl
TEST(Console_TEST, ColorDbgStdEndl)
{
  // Create a unique directory path
  boost::filesystem::path path = boost::filesystem::unique_path();

  // Initialize logging
  ignLogInit(path.string(), "test.log");

  // Get the absolute log file path
  boost::filesystem::path logPath = path / "test.log";

  std::string logString = "this is a dbg test";

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    igndbg << logString << " endl " << i << std::endl;
  }

  std::string logContent = GetLogContent(logPath.string());

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    std::ostringstream stream;
    stream << logString << " endl " << i;
    EXPECT_TRUE(logContent.find(stream.str()) != std::string::npos);
  }
}

//////////////////////////////////////////////////
/// \brief Test Console::ColorMsg with \n characters
TEST(Console_TEST, ColorMsgSlashN)
{
  // Create a unique directory path
  boost::filesystem::path path = boost::filesystem::unique_path();

  // Initialize logging
  ignLogInit(path.string(), "test.log");

  // Get the absolute log file path
  boost::filesystem::path logPath = path / "test.log";

  std::string logString = "this is a msg test";

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    ignmsg << logString << " _n__ " << i << '\n';
  }

  std::string logContent = GetLogContent(logPath.string());

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    std::ostringstream stream;
    stream << logString << " _n__ " << i;
    EXPECT_TRUE(logContent.find(stream.str()) != std::string::npos);
  }
}

//////////////////////////////////////////////////
/// \brief Test Console::ColorMsg with std::endl
TEST(Console_TEST, ColorMsgStdEndl)
{
  // Create a unique directory path
  boost::filesystem::path path = boost::filesystem::unique_path();

  // Initialize logging
  ignLogInit(path.string(), "test.log");

  // Get the absolute log file path
  boost::filesystem::path logPath = path / "test.log";

  std::string logString = "this is a msg test";

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    ignmsg << logString << " endl " << i << std::endl;
  }

  std::string logContent = GetLogContent(logPath.string());

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    std::ostringstream stream;
    stream << logString << " endl " << i;
    EXPECT_TRUE(logContent.find(stream.str()) != std::string::npos);
  }
}

//////////////////////////////////////////////////
/// \brief Test Console::ColorErr with \n characters
TEST(Console_TEST, ColorErrSlashN)
{
  // Create a unique directory path
  boost::filesystem::path path = boost::filesystem::unique_path();

  // Initialize logging
  ignLogInit(path.string(), "test.log");

  // Get the absolute log file path
  boost::filesystem::path logPath = path / "test.log";

  std::string logString = "this is an error test";

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    ignerr << logString << " _n__ " << i << '\n';
  }

  std::string logContent = GetLogContent(logPath.string());

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    std::ostringstream stream;
    stream << logString << " _n__ " << i;
    EXPECT_TRUE(logContent.find(stream.str()) != std::string::npos);
  }
}

//////////////////////////////////////////////////
/// \brief Test Console::ColorErr with std::endl
TEST(Console_TEST, ColorErrStdEndl)
{
  // Create a unique directory path
  boost::filesystem::path path = boost::filesystem::unique_path();

  // Initialize logging
  ignLogInit(path.string(), "test.log");

  // Get the absolute log file path
  boost::filesystem::path logPath = path / "test.log";

  std::string logString = "this is an error test";

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    ignerr << logString << " endl " << i << std::endl;
  }

  std::string logContent = GetLogContent(logPath.string());

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    std::ostringstream stream;
    stream << logString << " endl " << i;
    EXPECT_TRUE(logContent.find(stream.str()) != std::string::npos);
  }
}

/////////////////////////////////////////////////
/// \brief Test Console::ColorMsg
TEST(Console_TEST, ColorMsg)
{
  // Create a unique directory path
  boost::filesystem::path path = boost::filesystem::unique_path();

  // Initialize logging
  ignLogInit(path.string(), "test.log");

  // Get the absolute log file path
  boost::filesystem::path logPath = path / "test.log";

  std::string logString = "this is a msg test";

  ignmsg << logString << std::endl;

  std::string logContent = GetLogContent(logPath.string());

  EXPECT_TRUE(logContent.find(logString) != std::string::npos);
}

/////////////////////////////////////////////////
/// \brief Test Console::ColorErr
TEST(Console_TEST, ColorErr)
{
  // Create a unique directory path
  boost::filesystem::path path = boost::filesystem::unique_path();

  // Initialize logging
  ignLogInit(path.string(), "test.log");

  // Get the absolute log file path
  boost::filesystem::path logPath = path / "test.log";

  std::string logString = "this is an error test";

  ignerr << logString << std::endl;

  std::string logContent = GetLogContent(logPath.string());

  EXPECT_TRUE(logContent.find(logString) != std::string::npos);
}

/////////////////////////////////////////////////
/// \brief Test Console::LogDirectory
TEST(Console_TEST, LogDirectory)
{
  // Create a unique directory path
  boost::filesystem::path path = boost::filesystem::unique_path();

  // Initialize logging
  ignLogInit(path.string(), "test.log");

  std::string logDir = ignLogDirectory();

  // Get the absolute path
  boost::filesystem::path absPath = getenv("HOME");
  absPath /= path;

  EXPECT_EQ(logDir, absPath.string());
}

/////////////////////////////////////////////////
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
