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
#include <fstream>

#include <gtest/gtest.h>

#include "gz/common/testing/AutoLogFixture.hh"
#include "gz/common/testing/TestPaths.hh"

#include "ImageLoader.hh"
#include "ImageLoaderFreeImage.hh"
#include "ImageLoaderSTB.hh"

using namespace gz;

class ImageLoaderTest : public common::testing::AutoLogFixture { };

const std::string kTestDataGazeboJpeg =  // NOLINT(*)
    common::testing::TestFile("data", "gazebo_logo.jpeg");
const std::string kTestDataGazeboBmp =  // NOLINT(*)
    common::testing::TestFile("data", "gazebo_logo.bmp");
const std::string kTestData =  // NOLINT(*)
    common::testing::TestFile("data", "red_blue_colors.png");

using TestImageLoader = common::ImageLoaderSTB;

/////////////////////////////////////////////////
TEST_F(ImageLoaderTest, InvalidImage)
{
  auto loader = std::make_unique<TestImageLoader>();
  ASSERT_EQ(nullptr, loader->Load("/file/shouldn/never/exist.png"));
}

/////////////////////////////////////////////////
TEST_F(ImageLoaderTest, LoadBmp)
{
  auto loader = std::make_unique<TestImageLoader>();
  auto data = loader->Load(kTestDataGazeboBmp);
  ASSERT_NE(nullptr, data);

  EXPECT_EQ(kTestDataGazeboBmp, data->filename);
  EXPECT_EQ(554u, data->width);
  EXPECT_EQ(234u, data->height);
  EXPECT_EQ(common::Image::PixelFormatType::RGBA_INT8, data->pixel_format);
}

/////////////////////////////////////////////////
TEST_F(ImageLoaderTest, LoadJpg)
{
  auto loader = std::make_unique<TestImageLoader>();
  auto data = loader->Load(kTestDataGazeboJpeg);
  ASSERT_NE(nullptr, data);

  EXPECT_EQ(kTestDataGazeboJpeg, data->filename);
  EXPECT_EQ(554u, data->width);
  EXPECT_EQ(234u, data->height);
  EXPECT_EQ(common::Image::PixelFormatType::RGBA_INT8, data->pixel_format);
}

/////////////////////////////////////////////////
TEST_F(ImageLoaderTest, LoadPng)
{
  auto loader = std::make_unique<TestImageLoader>();
  ASSERT_NE(nullptr, loader->Load(kTestData));

}
