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
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "gz/common/CSVFile.hh"
#include "gz/common/Filesystem.hh"

using namespace gz;

/////////////////////////////////////////////////
TEST(CSVFileTests, StreamIteration)
{
  {
    std::stringstream sstream;
    EXPECT_EQ(common::CSVStreamIterator(sstream),
              common::CSVStreamIterator());
  }

  {
    std::stringstream sstream;
    sstream << "foo,bar,baz" << std::endl;
    const std::vector<std::vector<std::string>>
        expectedRows{{"foo", "bar", "baz"}};
    const auto rows = std::vector<std::vector<std::string>>(
        common::CSVStreamIterator(sstream), common::CSVStreamIterator());
    EXPECT_EQ(expectedRows, rows);
  }

  {
    std::stringstream sstream;
    sstream << ",bar,baz" << std::endl;
    const std::vector<std::vector<std::string>>
        expectedRows{{"", "bar", "baz"}};
    const auto rows = std::vector<std::vector<std::string>>(
        common::CSVStreamIterator(sstream), common::CSVStreamIterator());
    EXPECT_EQ(expectedRows, rows);
  }

  {
    std::stringstream sstream;
    sstream << "foo,bar," << std::endl;
    const std::vector<std::vector<std::string>>
        expectedRows{{"foo", "bar", ""}};
    const auto rows = std::vector<std::vector<std::string>>(
        common::CSVStreamIterator(sstream), common::CSVStreamIterator());
    EXPECT_EQ(expectedRows, rows);
  }

  {
    std::stringstream sstream;
    sstream << "foo,,baz" << std::endl;
    const std::vector<std::vector<std::string>>
        expectedRows{{"foo", "", "baz"}};
    const auto rows = std::vector<std::vector<std::string>>(
        common::CSVStreamIterator(sstream), common::CSVStreamIterator());
    EXPECT_EQ(expectedRows, rows);
  }

  {
    std::stringstream sstream;
    sstream << ",\"foo,bar,baz\"," << std::endl;
    const std::vector<std::vector<std::string>>
        expectedRows{{"", "foo,bar,baz", ""}};
    const auto rows = std::vector<std::vector<std::string>>(
        common::CSVStreamIterator(sstream), common::CSVStreamIterator());
    EXPECT_EQ(expectedRows, rows);
  }
}

/////////////////////////////////////////////////
TEST(CSVFileTests, EmptyFile)
{
  const std::string filePath = common::uniqueFilePath(
      std::filesystem::temp_directory_path() / "empty", "csv");
  std::fstream fs{filePath};
  fs.close();

  common::CSVFile file{filePath};
  EXPECT_EQ(filePath, file.Path());
  EXPECT_TRUE(file.Header().empty());
  common::CSVFile::Cursor cursor = file.Data();
  EXPECT_EQ(cursor.begin(), cursor.end());
  EXPECT_EQ(0, file.NumRows());
  EXPECT_EQ(0, file.NumColumns());
}

/////////////////////////////////////////////////
TEST(CSVFileTests, MissingFile)
{
  const std::string filePath = common::uniqueFilePath(
      std::filesystem::temp_directory_path() / "missing", "csv");
  (void)common::removeFile(filePath);

  EXPECT_THROW({
      common::CSVFile{filePath};
  }, std::runtime_error);
}

/////////////////////////////////////////////////
TEST(CSVFileTests, NominalFile)
{
  const std::string filePath = common::uniqueFilePath(
      std::filesystem::temp_directory_path() / "nominal", "csv");
  std::fstream fs{filePath};
  fs << "time,x,y,z,value" << std::endl
     << "10,0,0,0,1" << std::endl
     << "20,1,0,0,2" << std::endl
     << "30,0,0,1,3" << std::endl
     << "40,0,1,0,4" << std::endl
     << "50,1,1,1,5" << std::endl;
  fs.close();

  common::CSVFile file{filePath};
  EXPECT_EQ(filePath, file.Path());
  const std::vector<std::string>
      expectedHeader{"t", "x", "y", "z", "value"};
  EXPECT_EQ(expectedHeader, file.Header());
  common::CSVFile::Cursor cursor = file.Data();
  const std::vector<std::vector<std::string>>
      expectedData{{"10", "0", "0", "0", "1"},
                   {"20", "1", "0", "0", "2"},
                   {"30", "0", "0", "1", "3"},
                   {"40", "0", "1", "0", "4"},
                   {"50", "1", "1", "1", "5"}};
  const std::vector<std::vector<std::string>>
      data(cursor.begin(), cursor.end());
  EXPECT_EQ(expectedData, data);
  EXPECT_EQ(5, file.NumRows());
  EXPECT_EQ(5, file.NumColumns());
}
