/*
 * Copyright (C) 2014 Open Source Robotics Foundation
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
#ifndef IGNITION_TEST_UTIL_HH_
#define IGNITION_TEST_UTIL_HH_

#include <gtest/gtest.h>

#include "test_config.h"

#include <string>
#include "ignition/common/Console.hh"
#include "ignition/common/Filesystem.hh"
#include "ignition/common/Util.hh"

namespace ignition
{
  namespace testing
  {
    /// \brief A utility class that stores test logs in ~/.ignition/test_logs.
    /// This functionality is needed to keep all the log information reported
    /// by ignition during continuous integration. Without this, debugging
    /// failing tests is significantly more difficult.
    class AutoLogFixture : public ::testing::Test
    {
      /// \brief Setup the test fixture. This gets called by gtest.
      protected: virtual void SetUp()
      {
        const ::testing::TestInfo *const testInfo =
          ::testing::UnitTest::GetInstance()->current_test_info();

        std::string testName = testInfo->name();
        std::string testCaseName = testInfo->test_case_name();
        this->logFilename = testCaseName + "_" + testName + ".log";

        ignition::testing::TestSetHomePath(this->logBasePath);

        // Initialize Console
        ignLogInit(common::joinPaths(this->logBasePath, "test_logs"), 
                   this->logFilename);

        ignition::common::Console::SetVerbosity(4);

        // Read the full path to the log directory.
        this->logDirectory = ignLogDirectory();
      }

      /// \brief Get a string with the full log file path.
      /// \return The full log file path as a string.
      protected: std::string GetFullLogPath() const
      {
        return ignition::common::joinPaths(
          this->logDirectory, this->logFilename);
      }

      /// \brief Get a string with all the log content loaded from the disk.
      /// \return A string with all the log content.
      protected: std::string LogContent() const
      {
        std::string loggedString;
        std::cout << "2 Log Path[" <<this->GetFullLogPath() << "]\n";
        // Open the log file, and read back the string
        std::ifstream ifs(this->GetFullLogPath().c_str(), std::ios::in);

        while (!ifs.eof())
        {
          std::string line;
          std::getline(ifs, line);
          loggedString += line;
        }
        return loggedString;
      }

      /// \brief Default destructor.
      public: virtual ~AutoLogFixture()
      {
        ignLogClose();
        ignition::common::removeAll(this->logBasePath);
      }

      /// \brief String with the full path of the logfile
      private: std::string logFilename;

      /// \brief String with the full path to log directory
      private: std::string logDirectory;

      /// \brief String with the base path to log directory
      private: std::string logBasePath;
    };
  }
}
#endif
