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
#ifndef GZ_COMMON_TESTING_DETAIL_AUTOLOGFIXTURE_HH_
#define GZ_COMMON_TESTING_DETAIL_AUTOLOGFIXTURE_HH_

#include "gz/common/testing/AutoLogFixture.hh"

#include <memory>
#include <string>

#include <gz/common/Console.hh>
#include <gz/common/Filesystem.hh>
#include <gz/common/TempDirectory.hh>
#include <gz/common/Util.hh>

namespace ignition::common::testing
{

//////////////////////////////////////////////////
class AutoLogFixture::Implementation
{
  /// \brief String with the full path of the logfile
  public: std::string logFilename;

  /// \brief String with the full path to log directory
  public: std::string logDirectory;

  /// \brief String with the base path to log directory
  public: std::string logBasePath;

  /// \brief Temporary directory to run test in
  public: std::unique_ptr<common::TempDirectory> temp;
};


//////////////////////////////////////////////////
AutoLogFixture::AutoLogFixture():
  dataPtr(ignition::utils::MakeUniqueImpl<Implementation>())
{
}

//////////////////////////////////////////////////
AutoLogFixture::~AutoLogFixture()
{
  ignLogClose();
  EXPECT_TRUE(ignition::common::unsetenv(IGN_HOMEDIR));
}

//////////////////////////////////////////////////
void AutoLogFixture::SetUp()
{
  const ::testing::TestInfo *const testInfo =
    ::testing::UnitTest::GetInstance()->current_test_info();

  std::string testName = testInfo->name();
  std::string testCaseName = testInfo->test_case_name();
  this->dataPtr->logFilename = testCaseName + "_" + testName + ".log";

  std::replace(this->dataPtr->logFilename.begin(),
               this->dataPtr->logFilename.end(),
               '/', '_');

  this->dataPtr->temp = std::make_unique<TempDirectory>(
      "test", "ign_common", true);
  ASSERT_TRUE(this->dataPtr->temp->Valid());
  common::setenv(IGN_HOMEDIR, this->dataPtr->temp->Path());

  // Initialize Console
  auto logPath = common::joinPaths(this->dataPtr->temp->Path(), "test_logs");
  ignLogInit(logPath, this->dataPtr->logFilename);

  ASSERT_FALSE(logPath.empty());
  ASSERT_TRUE(common::exists(
        common::joinPaths(logPath, this->dataPtr->logFilename)));

  // Read the full path to the log directory.
  this->dataPtr->logDirectory = ignLogDirectory();
  ASSERT_FALSE(this->dataPtr->logDirectory.empty());
  ASSERT_TRUE(ignition::common::exists(this->dataPtr->logDirectory));

  ignition::common::Console::SetVerbosity(4);
}

//////////////////////////////////////////////////
std::string AutoLogFixture::FullLogPath() const
{
  return ignition::common::joinPaths(
    this->dataPtr->logDirectory, this->dataPtr->logFilename);
}

//////////////////////////////////////////////////
std::string AutoLogFixture::LogContent() const
{
  std::string loggedString;
  // Open the log file, and read back the string
  std::ifstream ifs(this->FullLogPath().c_str(), std::ios::in);

  while (!ifs.eof())
  {
    std::string line;
    std::getline(ifs, line);
    loggedString += line;
  }
  return loggedString;
}

}  // namespace ignition::common::testing

#endif  // GZ_COMMON_TESTING_DETAIL_AUTOLOGFIXTURE_HH_

