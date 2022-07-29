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
#include <sstream>
#include <string>
#include <vector>

#include "gz/common/CSVStreams.hh"

using namespace gz;

/////////////////////////////////////////////////
TEST(CSVStreams, CanIterateValidCSV)
{
  {
    std::stringstream ss;
    EXPECT_EQ(common::CSVIStreamIterator(ss),
              common::CSVIStreamIterator());
    EXPECT_FALSE(ss.fail());
  }

  {
    std::stringstream ss;
    ss << "foo,bar,baz" << std::endl;
    const std::vector<std::vector<std::string>>
        expectedRows{{"foo", "bar", "baz"}};
    const auto rows = std::vector<std::vector<std::string>>(
        common::CSVIStreamIterator(ss), common::CSVIStreamIterator());
    EXPECT_EQ(expectedRows, rows);
    EXPECT_FALSE(ss.fail());
  }

  {
    std::stringstream ss;
    ss << ",bar,baz" << std::endl;
    const std::vector<std::vector<std::string>>
        expectedRows{{"", "bar", "baz"}};
    const auto rows = std::vector<std::vector<std::string>>(
        common::CSVIStreamIterator(ss), common::CSVIStreamIterator());
    EXPECT_EQ(expectedRows, rows);
    EXPECT_FALSE(ss.fail());
  }

  {
    std::stringstream ss;
    ss << "foo,bar," << std::endl;
    const std::vector<std::vector<std::string>>
        expectedRows{{"foo", "bar", ""}};
    const auto rows = std::vector<std::vector<std::string>>(
        common::CSVIStreamIterator(ss), common::CSVIStreamIterator());
    EXPECT_EQ(expectedRows, rows);
    EXPECT_FALSE(ss.fail());
  }

  {
    std::stringstream ss;
    ss << "foo,,baz" << std::endl;
    const std::vector<std::vector<std::string>>
        expectedRows{{"foo", "", "baz"}};
    const auto rows = std::vector<std::vector<std::string>>(
        common::CSVIStreamIterator(ss), common::CSVIStreamIterator());
    EXPECT_EQ(expectedRows, rows);
    EXPECT_FALSE(ss.fail());
  }

  {
    std::stringstream ss;
    ss << ",\"foo,bar,baz\"," << std::endl;
    const std::vector<std::vector<std::string>>
        expectedRows{{"", "foo,bar,baz", ""}};
    const auto rows = std::vector<std::vector<std::string>>(
        common::CSVIStreamIterator(ss), common::CSVIStreamIterator());
    EXPECT_EQ(expectedRows, rows);
    EXPECT_FALSE(ss.fail());
  }

  {
    std::stringstream ss;
    ss << "foo,," << std::endl;
    const std::vector<std::vector<std::string>>
        expectedRows{{"foo", "", ""}};
    const auto rows = std::vector<std::vector<std::string>>(
        common::CSVIStreamIterator(ss), common::CSVIStreamIterator());
    EXPECT_EQ(expectedRows, rows);
    EXPECT_FALSE(ss.fail());
  }
}

/////////////////////////////////////////////////
TEST(CSVStreams, CanHandleInvalidCSV)
{
  {
    std::stringstream ss;
    ss << "foo,bar,b\"az" << std::endl;
    const auto rows = std::vector<std::vector<std::string>>(
        common::CSVIStreamIterator(ss), common::CSVIStreamIterator());
    EXPECT_TRUE(rows.empty());
    EXPECT_TRUE(ss.fail());
  }

  {
    std::stringstream ss;
    ss.exceptions(std::stringstream::failbit);
    ss << "foo,bar,\"baz" << std::endl;
    EXPECT_THROW({
        const auto rows = std::vector<std::vector<std::string>>(
            common::CSVIStreamIterator(ss), common::CSVIStreamIterator());
        (void)rows;
    }, std::stringstream::failure);
  }
}
