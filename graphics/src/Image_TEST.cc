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

using namespace ignition;

class ImageTest : public common::testing::AutoLogFixture { };

/////////////////////////////////////////////////
TEST_F(ImageTest, Image)
{
  common::Image img;
  EXPECT_EQ(-1, img.Load("/file/shouldn/never/exist.png"));

  std::string filename =  common::testing::TestFile(
      "data", "red_blue_colors.png");

  // load image and test colors
  ASSERT_EQ(0, img.Load(filename));
  ASSERT_TRUE(img.Valid());

  EXPECT_EQ(121u, img.Width());
  EXPECT_EQ(81u, img.Height());
  EXPECT_EQ(32u, img.BPP());
  EXPECT_EQ(484, img.Pitch());
  EXPECT_EQ(common::Image::PixelFormatType::RGBA_INT8, img.PixelFormat());
  EXPECT_EQ(img.Pixel(0, 0), math::Color::Red);
  EXPECT_EQ(img.Pixel(85, 0), math::Color::Blue);
  EXPECT_EQ(img.AvgColor(), math::Color(0.661157f, 0, 0.338843f, 1));
  EXPECT_EQ(img.MaxColor(), math::Color::Red);
  EXPECT_TRUE(img.Filename().find("red_blue_colors.png") !=
      std::string::npos);

  // Check RGB data
  unsigned char *data = nullptr;
  unsigned int size = 0;
  img.RGBData(&data, size);
  EXPECT_EQ(29403u, size);
  ASSERT_NE(nullptr, data);

  auto channels = 3u;
  auto step = img.Width() * channels;
  for (auto i = 0u; i < img.Height(); ++i)
  {
    for (auto j = 0u; j < step; j += channels)
    {
      unsigned int idx = i * step + j;
      unsigned int r = data[idx];
      unsigned int g = data[idx+1];
      unsigned int b = data[idx+2];

      EXPECT_EQ(0u, g) << i << "  " << j;
      if (j / channels < 80)
      {
        EXPECT_EQ(255u, r) << i << "  " << j / channels;
        EXPECT_EQ(0u, b) << i << "  " << j / channels;
      }
      else
      {
        EXPECT_EQ(0u, r) << i << "  " << j / channels;
        EXPECT_EQ(255u, b) << i << "  " << j / channels;
      }
    }
  }

  // Check RGBA data
  data = nullptr;
  size = 0;
  img.Data(&data, size);
  EXPECT_EQ(39204u, size);
  ASSERT_NE(nullptr, data);

  channels = 4u;
  step = img.Width() * channels;
  for (auto i = 0u; i < img.Height(); ++i)
  {
    for (auto j = 0u; j < step; j += channels)
    {
      unsigned int idx = i * step + j;
      unsigned int r = data[idx];
      unsigned int g = data[idx+1];
      unsigned int b = data[idx+2];
      unsigned int a = data[idx+3];

      EXPECT_EQ(0u, g) << i << "  " << j;
      EXPECT_EQ(255u, a) << i << "  " << j;
      if (j / channels < 80)
      {
        EXPECT_EQ(255u, r) << i << "  " << j / channels;
        EXPECT_EQ(0u, b) << i << "  " << j / channels;
      }
      else
      {
        EXPECT_EQ(0u, r) << i << "  " << j / channels;
        EXPECT_EQ(255u, b) << i << "  " << j / channels;
      }
    }
  }

  // Set from RGBA data
  img.SetFromData(data, img.Width(), img.Height(), img.PixelFormat());
  ASSERT_TRUE(img.Valid());

  EXPECT_EQ(common::Image::PixelFormatType::RGBA_INT8, img.PixelFormat());
  EXPECT_EQ(121u, img.Width());
  EXPECT_EQ(81u, img.Height());
  EXPECT_EQ(32u, img.BPP());
  EXPECT_EQ(484, img.Pitch());
  EXPECT_EQ(img.Pixel(0, 0), math::Color::Red);
  EXPECT_EQ(img.Pixel(85, 0), math::Color::Blue);
  EXPECT_EQ(img.AvgColor(), math::Color(0.661157f, 0, 0.338843f, 1));
  EXPECT_EQ(img.MaxColor(), math::Color::Red);

  // save image then reload and test colors
  std::string testSaveImage =
    common::testing::TempPath("test_red_blue_save.png");
  std::cout << testSaveImage << std::endl;
  img.SavePNG(testSaveImage);

  img.Load("file://" + testSaveImage);
  ASSERT_TRUE(img.Valid());
  EXPECT_EQ(common::Image::PixelFormatType::RGB_INT8, img.PixelFormat());
  EXPECT_EQ(121u, img.Width());
  EXPECT_EQ(81u, img.Height());
  EXPECT_EQ(24u, img.BPP());
  EXPECT_EQ(363, img.Pitch());
  EXPECT_EQ(img.Pixel(0, 0), math::Color::Red);
  EXPECT_EQ(img.Pixel(85, 0), math::Color::Blue);
  EXPECT_EQ(img.AvgColor(), math::Color(0.661157f, 0, 0.338843f, 1));
  EXPECT_EQ(img.MaxColor(), math::Color::Red);

  // Check data
  data = nullptr;
  size = 0;
  img.Data(&data, size);
  EXPECT_EQ(29403u, size);
  ASSERT_NE(nullptr, data);

  channels = 3u;
  step = img.Width() * channels;
  for (auto i = 0u; i < img.Height(); ++i)
  {
    for (auto j = 0u; j < step; j += channels)
    {
      unsigned int idx = i * step + j;
      unsigned int r = data[idx];
      unsigned int g = data[idx+1];
      unsigned int b = data[idx+2];

      EXPECT_EQ(0u, g) << i << "  " << j;
      if (j / channels < 80)
      {
        EXPECT_EQ(255u, r) << i << "  " << j / channels;
        EXPECT_EQ(0u, b) << i << "  " << j / channels;
      }
      else
      {
        EXPECT_EQ(0u, r) << i << "  " << j / channels;
        EXPECT_EQ(255u, b) << i << "  " << j / channels;
      }
    }
  }

  img.SetFromData(data, img.Width(), img.Height(), img.PixelFormat());
  ASSERT_TRUE(img.Valid());

  EXPECT_EQ(common::Image::PixelFormatType::RGB_INT8, img.PixelFormat());
  EXPECT_EQ(121u, img.Width());
  EXPECT_EQ(81u, img.Height());
  EXPECT_EQ(24u, img.BPP());
  EXPECT_EQ(363, img.Pitch());
  EXPECT_EQ(img.Pixel(0, 0), math::Color::Red);
  EXPECT_EQ(img.Pixel(85, 0), math::Color::Blue);
  EXPECT_EQ(img.AvgColor(), math::Color(0.661157f, 0, 0.338843f, 1));
  EXPECT_EQ(img.MaxColor(), math::Color::Red);

  common::removeDirectoryOrFile(filename);
}

/////////////////////////////////////////////////
TEST_F(ImageTest, ConvertPixelFormat)
{
  using Image = ignition::common::Image;
  EXPECT_EQ(Image::PixelFormatType::UNKNOWN_PIXEL_FORMAT,
         Image::ConvertPixelFormat("fake"));
  EXPECT_EQ(Image::PixelFormatType::UNKNOWN_PIXEL_FORMAT,
         Image::ConvertPixelFormat("unknown"));
  EXPECT_EQ(Image::PixelFormatType::UNKNOWN_PIXEL_FORMAT,
         Image::ConvertPixelFormat("UNKNOWN_PIXEL_FORMAT"));
  EXPECT_EQ(Image::PixelFormatType::L_INT8,
         Image::ConvertPixelFormat("L_INT8"));
  EXPECT_EQ(Image::PixelFormatType::L_INT16,
         Image::ConvertPixelFormat("L_INT16"));
  EXPECT_EQ(Image::PixelFormatType::RGB_INT8,
         Image::ConvertPixelFormat("RGB_INT8"));
  EXPECT_EQ(Image::PixelFormatType::RGBA_INT8,
         Image::ConvertPixelFormat("RGBA_INT8"));
  EXPECT_EQ(Image::PixelFormatType::RGB_INT16,
         Image::ConvertPixelFormat("RGB_INT16"));
  EXPECT_EQ(Image::PixelFormatType::RGB_INT32,
         Image::ConvertPixelFormat("RGB_INT32"));
  EXPECT_EQ(Image::PixelFormatType::BGR_INT8,
         Image::ConvertPixelFormat("BGR_INT8"));
  EXPECT_EQ(Image::PixelFormatType::BGRA_INT8,
         Image::ConvertPixelFormat("BGRA_INT8"));
  EXPECT_EQ(Image::PixelFormatType::BGR_INT16,
         Image::ConvertPixelFormat("BGR_INT16"));
  EXPECT_EQ(Image::PixelFormatType::BGR_INT32,
         Image::ConvertPixelFormat("BGR_INT32"));
  EXPECT_EQ(Image::PixelFormatType::R_FLOAT16,
         Image::ConvertPixelFormat("R_FLOAT16"));
  EXPECT_EQ(Image::PixelFormatType::R_FLOAT32,
         Image::ConvertPixelFormat("R_FLOAT32"));
  EXPECT_EQ(Image::PixelFormatType::RGB_FLOAT16,
         Image::ConvertPixelFormat("RGB_FLOAT16"));
  EXPECT_EQ(Image::PixelFormatType::RGB_FLOAT32,
         Image::ConvertPixelFormat("RGB_FLOAT32"));
  EXPECT_EQ(Image::PixelFormatType::BAYER_RGGB8,
         Image::ConvertPixelFormat("BAYER_RGGB8"));
  EXPECT_EQ(Image::PixelFormatType::BAYER_RGGR8,
         Image::ConvertPixelFormat("BAYER_RGGR8"));
  EXPECT_EQ(Image::PixelFormatType::BAYER_GBRG8,
         Image::ConvertPixelFormat("BAYER_GBRG8"));
  EXPECT_EQ(Image::PixelFormatType::BAYER_GRBG8,
         Image::ConvertPixelFormat("BAYER_GRBG8"));
  EXPECT_EQ(Image::PixelFormatType::BAYER_BGGR8,
         Image::ConvertPixelFormat("BAYER_BGGR8"));
}

using string_int2 = std::tuple<const char *, unsigned int, unsigned int>;

class ImagePerformanceTest : public ImageTest,
                             public ::testing::WithParamInterface<string_int2>
{
  /// \brief Loads an image specified by _filename and checks its max color.
  /// \param[in] _filePath Relative path to the image file to check.
  /// \param[in] _width Width of the image in pixels.
  /// \param[in] _height Height of the image in pixels.
  public: void MaxColor(const std::string &_filePath,
                        const unsigned int _width,
                        const unsigned int _height);
};

TEST_P(ImagePerformanceTest, MaxColorFlatHeightmap)
{
  MaxColor(std::get<0>(GetParam()), std::get<1>(GetParam()),
           std::get<2>(GetParam()));
}

void ImagePerformanceTest::MaxColor(const std::string &_filePath,
                                    const unsigned int _width,
                                    const unsigned int _height)
{
  std::string fileName = common::testing::TestFile("data", _filePath);

  common::Image img;
  EXPECT_EQ(0, img.Load(fileName));
  EXPECT_EQ(_width, img.Width());
  EXPECT_EQ(_height, img.Height());
  EXPECT_EQ(ignition::math::Color(0., 0., 0., 0), img.MaxColor());
  EXPECT_TRUE(img.Valid());
}

INSTANTIATE_TEST_SUITE_P(FlatHeightmaps, ImagePerformanceTest,
  ::testing::Values(
    std::make_tuple("heightmap_flat_129x129.png", 129u, 129u),
    std::make_tuple("heightmap_flat_257x257.png", 257u, 257u),
    std::make_tuple("heightmap_flat_513x513.png", 513u, 513u),
    std::make_tuple("heightmap_flat_1025x1025.png", 1025u, 1025u)));

/////////////////////////////////////////////////
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
