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

#include <gtest/gtest.h>
#include <stdlib.h>

#include "ignition/common/Console.hh"
#include "ignition/common/Filesystem.hh"
#include "ignition/common/Time.hh"
#include "ignition/common/Util.hh"

#include "test/util.hh"

#ifndef _WIN32
const int g_messageRepeat = 4;

class Console_TEST : public ::testing::Test {
  /// \brief Clear out all the directories we produced during this test.
  public: virtual ~Console_TEST()
  {
    std::string absPath;
    ignition::common::env(IGN_HOMEDIR, absPath);
    absPath = ignition::common::joinPaths(absPath, std::string(IGN_TMP_DIR));

    if (ignition::common::isDirectory(absPath))
    {
      EXPECT_TRUE(ignition::common::removeAll(absPath));
    }
  }
};

std::string GetLogContent(const std::string &_filename)
{
  // Get the absolute path
  std::string path;
  EXPECT_TRUE(ignition::common::env(IGN_HOMEDIR, path));
  path = ignition::common::joinPaths(path, _filename);

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
/// \brief Test Console::Init and Console::Log
TEST_F(Console_TEST, NoInitAndLog)
{
  // Log the string
  std::string logString = "this is a test";
  ignlog << logString << std::endl;

  // Get the absolute log file path
  std::string logPath = ".ignition/auto_default.log";

  // Expect to find the string in the log file
  EXPECT_TRUE(GetLogContent(logPath).find(logString) != std::string::npos);

  // Cleanup
  std::string path;
  EXPECT_TRUE(ignition::common::env(IGN_HOMEDIR, path));
  path = ignition::common::joinPaths(path, logPath);
  ignition::common::removeAll(path);
}

/////////////////////////////////////////////////
/// \brief Test Console::Init and Console::Log
TEST_F(Console_TEST, InitAndLog)
{
  // Create a unique directory path
  std::string path = IGN_TMP_DIR + ignition::common::uuid();

  // Initialize logging
  ignLogInit(path, "test.log");

  // Log the string
  std::string logString = "this is a test";
  ignlog << logString << std::endl;

  // Get the absolute path
  std::string basePath;
  EXPECT_TRUE(ignition::common::env(IGN_HOMEDIR, basePath));
  basePath = ignition::common::joinPaths(basePath, path);

  // Get the absolute log file path
  std::string logPath = ignition::common::joinPaths(path, "test.log");

  // Expect to find the string in the log file
  EXPECT_TRUE(GetLogContent(logPath).find(logString) != std::string::npos);

  // Cleanup
  ignition::common::removeAll(basePath);
}

//////////////////////////////////////////////////
/// \brief Test Console::Log with \n characters
TEST_F(Console_TEST, LogSlashN)
{
  // Create a unique directory path
  std::string path = IGN_TMP_DIR + ignition::common::uuid();

  // Initialize logging
  ignLogInit(path, "test.log");

  // Get the absolute log file path
  std::string logPath = ignition::common::joinPaths(path, "test.log");

  std::string logString = "this is a log test";

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    ignlog << logString << " _n__ " << i << '\n';
  }

  std::string logContent = GetLogContent(logPath);

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    std::ostringstream stream;
    stream << logString << " _n__ " << i;
    EXPECT_TRUE(logContent.find(stream.str()) != std::string::npos);
  }
}

//////////////////////////////////////////////////
/// \brief Test Console::Log with std::endl
TEST_F(Console_TEST, LogStdEndl)
{
  // Create a unique directory path
  std::string path = IGN_TMP_DIR + ignition::common::uuid();

  // Initialize logging
  ignLogInit(path, "test.log");

  // Get the absolute log file path
  std::string logPath = ignition::common::joinPaths(path, "test.log");

  std::string logString = "this is a log test";

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    ignlog << logString << " endl " << i << std::endl;
  }

  std::string logContent = GetLogContent(logPath);

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    std::ostringstream stream;
    stream << logString << " endl " << i;
    EXPECT_TRUE(logContent.find(stream.str()) != std::string::npos);
  }
}

//////////////////////////////////////////////////
/// \brief Test Console::ColorWarn with \n characters
TEST_F(Console_TEST, ColorWarnSlashN)
{
  // Create a unique directory path
  std::string path = ignition::common::joinPaths(
        IGN_TMP_DIR, ignition::common::uuid());

  // Initialize logging
  ignLogInit(path, "test.log");

  // Get the absolute log file path
  std::string logPath = ignition::common::joinPaths(path, "test.log");

  std::string logString = "this is a warning test";

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    ignwarn << logString << " _n__ " << i << '\n';
  }

  std::string logContent = GetLogContent(logPath);

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    std::ostringstream stream;
    stream << logString << " _n__ " << i;
    EXPECT_TRUE(logContent.find(stream.str()) != std::string::npos);
  }
}

//////////////////////////////////////////////////
/// \brief Test Console::ColorWarn with std::endl
TEST_F(Console_TEST, ColorWarnStdEndl)
{
  // Create a unique directory path
  std::string path = ignition::common::joinPaths(
        IGN_TMP_DIR, ignition::common::uuid());

  // Initialize logging
  ignLogInit(path, "test.log");

  // Get the absolute log file path
  std::string logPath = ignition::common::joinPaths(path, "test.log");

  std::string logString = "this is a warning test";

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    ignwarn << logString << " endl " << i << std::endl;
  }

  std::string logContent = GetLogContent(logPath);

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    std::ostringstream stream;
    stream << logString << " endl " << i;
    EXPECT_TRUE(logContent.find(stream.str()) != std::string::npos);
  }
}

//////////////////////////////////////////////////
/// \brief Test Console::ColorDbg with \n characters
TEST_F(Console_TEST, ColorDbgSlashN)
{
  // Create a unique directory path
  std::string path = ignition::common::joinPaths(
        IGN_TMP_DIR, ignition::common::uuid());

  // Initialize logging
  ignLogInit(path, "test.log");

  // Get the absolute log file path
  std::string logPath = ignition::common::joinPaths(path, "test.log");

  std::string logString = "this is a dbg test";

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    igndbg << logString << " _n__ " << i << '\n';
  }

  std::string logContent = GetLogContent(logPath);

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    std::ostringstream stream;
    stream << logString << " _n__ " << i;
    EXPECT_TRUE(logContent.find(stream.str()) != std::string::npos);
  }
}

//////////////////////////////////////////////////
/// \brief Test Console::ColorDbg with std::endl
TEST_F(Console_TEST, ColorDbgStdEndl)
{
  // Create a unique directory path
  std::string path = ignition::common::joinPaths(
        IGN_TMP_DIR, ignition::common::uuid());

  // Initialize logging
  ignLogInit(path, "test.log");

  // Get the absolute log file path
  std::string logPath = ignition::common::joinPaths(path, "test.log");

  std::string logString = "this is a dbg test";

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    igndbg << logString << " endl " << i << std::endl;
  }

  std::string logContent = GetLogContent(logPath);

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    std::ostringstream stream;
    stream << logString << " endl " << i;
    EXPECT_TRUE(logContent.find(stream.str()) != std::string::npos);
  }
}

//////////////////////////////////////////////////
/// \brief Test Console::ColorMsg with \n characters
TEST_F(Console_TEST, ColorMsgSlashN)
{
  // Create a unique directory path
  std::string path = ignition::common::joinPaths(
        IGN_TMP_DIR, ignition::common::uuid());

  // Initialize logging
  ignLogInit(path, "test.log");

  // Get the absolute log file path
  std::string logPath = ignition::common::joinPaths(path, "test.log");

  std::string logString = "this is a msg test";

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    ignmsg << logString << " _n__ " << i << '\n';
  }

  std::string logContent = GetLogContent(logPath);

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    std::ostringstream stream;
    stream << logString << " _n__ " << i;
    EXPECT_TRUE(logContent.find(stream.str()) != std::string::npos);
  }
}

//////////////////////////////////////////////////
/// \brief Test Console::ColorMsg with std::endl
TEST_F(Console_TEST, ColorMsgStdEndl)
{
  // Create a unique directory path
  std::string path = ignition::common::joinPaths(
        IGN_TMP_DIR, ignition::common::uuid());

  // Initialize logging
  ignLogInit(path, "test.log");

  // Get the absolute log file path
  std::string logPath = ignition::common::joinPaths(path, "test.log");

  std::string logString = "this is a msg test";

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    ignmsg << logString << " endl " << i << std::endl;
  }

  std::string logContent = GetLogContent(logPath);

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    std::ostringstream stream;
    stream << logString << " endl " << i;
    EXPECT_TRUE(logContent.find(stream.str()) != std::string::npos);
  }
}

//////////////////////////////////////////////////
/// \brief Test Console::ColorErr with \n characters
TEST_F(Console_TEST, ColorErrSlashN)
{
  // Create a unique directory path
  std::string path = ignition::common::joinPaths(
        IGN_TMP_DIR, ignition::common::uuid());

  // Initialize logging
  ignLogInit(path, "test.log");

  // Get the absolute log file path
  std::string logPath = ignition::common::joinPaths(path, "test.log");

  std::string logString = "this is an error test";

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    ignerr << logString << " _n__ " << i << '\n';
  }

  std::string logContent = GetLogContent(logPath);

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    std::ostringstream stream;
    stream << logString << " _n__ " << i;
    EXPECT_TRUE(logContent.find(stream.str()) != std::string::npos);
  }
}

//////////////////////////////////////////////////
/// \brief Test Console::ColorErr with std::endl
TEST_F(Console_TEST, ColorErrStdEndl)
{
  // Create a unique directory path
  std::string path = ignition::common::joinPaths(
        IGN_TMP_DIR, ignition::common::uuid());

  // Initialize logging
  ignLogInit(path, "test.log");

  // Get the absolute log file path
  std::string logPath = ignition::common::joinPaths(path, "test.log");

  std::string logString = "this is an error test";

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    ignerr << logString << " endl " << i << std::endl;
  }

  std::string logContent = GetLogContent(logPath);

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    std::ostringstream stream;
    stream << logString << " endl " << i;
    EXPECT_TRUE(logContent.find(stream.str()) != std::string::npos);
  }
}

/////////////////////////////////////////////////
/// \brief Test Console::ColorMsg
TEST_F(Console_TEST, ColorMsg)
{
  // Create a unique directory path
  std::string path = ignition::common::joinPaths(
        IGN_TMP_DIR, ignition::common::uuid());

  // Initialize logging
  ignLogInit(path, "test.log");

  // Get the absolute log file path
  std::string logPath = ignition::common::joinPaths(path, "test.log");

  std::string logString = "this is a msg test";

  ignmsg << logString << std::endl;

  std::string logContent = GetLogContent(logPath);

  EXPECT_TRUE(logContent.find(logString) != std::string::npos);
}

/////////////////////////////////////////////////
/// \brief Test Console::ColorErr
TEST_F(Console_TEST, ColorErr)
{
  // Create a unique directory path
  std::string path = ignition::common::joinPaths(
        IGN_TMP_DIR, ignition::common::uuid());

  // Initialize logging
  ignLogInit(path, "test.log");

  // Get the absolute log file path
  std::string logPath = ignition::common::joinPaths(path, "test.log");

  std::string logString = "this is an error test";

  ignerr << logString << std::endl;

  std::string logContent = GetLogContent(logPath);

  EXPECT_TRUE(logContent.find(logString) != std::string::npos);
}

/////////////////////////////////////////////////
/// \brief Test Console::Verbosity
TEST_F(Console_TEST, Verbosity)
{
  EXPECT_EQ(ignition::common::Console::Verbosity(), 1);

  ignition::common::Console::SetVerbosity(2);
  EXPECT_EQ(ignition::common::Console::Verbosity(), 2);

  ignition::common::Console::SetVerbosity(-1);
  EXPECT_EQ(ignition::common::Console::Verbosity(), -1);
}

/////////////////////////////////////////////////
/// \brief Test Console::Prefix
TEST_F(Console_TEST, Prefix)
{
  // Max verbosity
  ignition::common::Console::SetVerbosity(4);

  // Path to log file
  std::string path = ignition::common::joinPaths(
        IGN_TMP_DIR, ignition::common::uuid());

  ignLogInit(path, "test.log");
  std::string logPath = ignition::common::joinPaths(path, "test.log");

  // Check default prefix
  EXPECT_EQ(ignition::common::Console::Prefix(), "");

  // Set new prefix
  ignition::common::Console::SetPrefix("**test** ");
  EXPECT_EQ(ignition::common::Console::Prefix(), "**test** ");

  // Use the console
  ignerr << "error" << std::endl;
  ignwarn << "warning" << std::endl;
  ignmsg << "message" << std::endl;
  igndbg << "debug" << std::endl;

  // Get the logged content
  std::string logContent = GetLogContent(logPath);

  // Check
  EXPECT_TRUE(logContent.find("**test** [Err]") != std::string::npos);
  EXPECT_TRUE(logContent.find("**test** [Wrn]") != std::string::npos);
  EXPECT_TRUE(logContent.find("**test** [Msg]") != std::string::npos);
  EXPECT_TRUE(logContent.find("**test** [Dbg]") != std::string::npos);

  // Reset
  ignition::common::Console::SetPrefix("");
  EXPECT_EQ(ignition::common::Console::Prefix(), "");
}

/////////////////////////////////////////////////
/// \brief Test Console::LogDirectory
TEST_F(Console_TEST, LogDirectory)
{
  // Create a unique directory path
  std::string path = ignition::common::joinPaths(
        IGN_TMP_DIR, ignition::common::uuid());

  // Initialize logging
  ignLogInit(path, "test.log");

  std::string logDir = ignLogDirectory();

  // Get the absolute path
  std::string absPath;
  EXPECT_TRUE(ignition::common::env(IGN_HOMEDIR, absPath));
  absPath = ignition::common::joinPaths(absPath, path);

  EXPECT_EQ(logDir, absPath);
}

/////////////////////////////////////////////////
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
#endif
