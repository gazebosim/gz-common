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

#include <ignition/common/Image.hh>
#include "test_config.h"
#include "test/util.hh"

using namespace ignition;

class ImageTest : public ignition::testing::AutoLogFixture { };

TEST_F(ImageTest, Image)
{
  common::Image img;
  EXPECT_EQ(-1, img.Load("/file/shouldn/never/exist.png"));
  std::string filename =  "file://";
  filename += PROJECT_SOURCE_PATH;
  filename += "/test/data/cordless_drill/materials/textures/cordless_drill.png";
  EXPECT_EQ(0, img.Load(filename));
  EXPECT_EQ(static_cast<unsigned int>(128), img.Width());
  EXPECT_EQ(static_cast<unsigned int>(128), img.Height());
  EXPECT_EQ(static_cast<unsigned int>(32), img.BPP());
  EXPECT_TRUE(img.Pixel(10, 10) ==
      math::Color(0.141176, 0.172549, 0.133333, 1));
  EXPECT_TRUE(img.AvgColor() ==
      math::Color(0.259651, 0.271894, 0.414959, 1));
  EXPECT_TRUE(img.MaxColor() ==
      math::Color(0.929412, 0.921569, 0.917647, 1));
  EXPECT_TRUE(img.Valid());
  EXPECT_TRUE(img.Filename().find("cordless_drill.png") !=
      std::string::npos);

  unsigned char *data = NULL;
  unsigned int size = 0;
  img.Data(&data, size);
  EXPECT_EQ(static_cast<unsigned int>(65536), size);

  img.SetFromData(data, img.Width(), img.Height(),
                  common::Image::RGB_INT8);
}


/////////////////////////////////////////////////
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
