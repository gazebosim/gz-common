/*
 * Copyright (C) 2025 Open Source Robotics Foundation
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

#include "gz/common/geospatial/HeightmapLoader.hh"

#include "gz/common/testing/AutoLogFixture.hh"
#include "gz/common/testing/TestPaths.hh"

using namespace gz;

class HeightmapLoaderTest : public common::testing::AutoLogFixture { };

/////////////////////////////////////////////////
TEST_F(HeightmapLoaderTest, SupportedImageFileExtension)
{
  EXPECT_TRUE(common::HeightmapLoader::SupportedImageFileExtension("f.jpg"));
  EXPECT_TRUE(common::HeightmapLoader::SupportedImageFileExtension("f.jpeg"));
  EXPECT_TRUE(common::HeightmapLoader::SupportedImageFileExtension("f.png"));
  EXPECT_TRUE(common::HeightmapLoader::SupportedImageFileExtension("f.JPG"));
  EXPECT_TRUE(common::HeightmapLoader::SupportedImageFileExtension("f.JPEG"));
  EXPECT_TRUE(common::HeightmapLoader::SupportedImageFileExtension("f.PNG"));
  EXPECT_TRUE(common::HeightmapLoader::SupportedImageFileExtension(".PNG"));
  EXPECT_FALSE(common::HeightmapLoader::SupportedImageFileExtension("f.tiff"));
  EXPECT_FALSE(common::HeightmapLoader::SupportedImageFileExtension("invalid"));
  EXPECT_FALSE(common::HeightmapLoader::SupportedImageFileExtension(""));
}

/////////////////////////////////////////////////
TEST_F(HeightmapLoaderTest, LoadImage)
{
  const auto path = common::testing::TestFile("data", "heightmap_bowl.png");
  std::unique_ptr<common::HeightmapData> data =
      common::HeightmapLoader::Load(path);
  EXPECT_NE(nullptr, data);
}

/////////////////////////////////////////////////
TEST_F(HeightmapLoaderTest, LoadDEM)
{
  const auto path = common::testing::TestFile("data", "dem_squared.tif");
  std::unique_ptr<common::HeightmapData> data =
      common::HeightmapLoader::Load(path);
  EXPECT_NE(nullptr, data);
}

/////////////////////////////////////////////////
TEST_F(HeightmapLoaderTest, LoadInvalidFile)
{
  EXPECT_EQ(nullptr, common::HeightmapLoader::Load("invalid/file.jpg"));
  EXPECT_EQ(nullptr, common::HeightmapLoader::Load("invalid/file_no_extension"));
}
