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
#ifndef IGNITION_COMMON_TESTING_AUTOLOGFIXTURE_HH_
#define IGNITION_COMMON_TESTING_AUTOLOGFIXTURE_HH_

#include <gtest/gtest.h>

#include <memory>
#include <string>

#include <ignition/utils/ImplPtr.hh>

namespace ignition::common::testing
{
/// \brief A utility class that stores test logs in ~/.ignition/test_logs.
/// This functionality is needed to keep all the log information reported
/// by ignition during continuous integration. Without this, debugging
/// failing tests is significantly more difficult.
class AutoLogFixture : public ::testing::Test
{
  /// \brief Constructor
  public: AutoLogFixture();

  /// \brief Destructor
  public: virtual ~AutoLogFixture();

  /// \brief Setup the test fixture. This gets called by gtest.
  protected: virtual void SetUp() override;

  /// \brief Get a string with the full log file path.
  /// \return The full log file path as a string.
  protected: std::string FullLogPath() const;

  /// \brief Get a string with all the log content loaded from the disk.
  /// \return A string with all the log content.
  protected: std::string LogContent() const;

  /// \brief Pointer to private data.
  IGN_UTILS_UNIQUE_IMPL_PTR(dataPtr)
};
}  // namespace ignition::common::testing

#endif  // IGNITION_COMMON_TESTING_AUTOLOGFIXTURE_HH_
