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


const std::string kTestData =  // NOLINT(*)
    common::testing::TestFile("data", "red_blue_colors.png");

const auto kWidth = 121u;
const auto kHeight = 81u;

const auto kBpp_RGB = 24u;
const int kPitch_RGB = kWidth * kBpp_RGB/8;
const unsigned kSize_RGB = kPitch_RGB * kHeight;

const auto kBpp_RGBA = 32u;
const int kPitch_RGBA = kWidth * kBpp_RGBA/8;
const unsigned kSize_RGBA = kPitch_RGBA * kHeight;

const auto kAvgColor = math::Color(0.661157f, 0, 0.338843f, 1);
const auto kMaxColor = math::Color::Red;

/////////////////////////////////////////////////
void CheckImageRGB(const common::Image &_img)
{
  ASSERT_TRUE(_img.Valid());
  ASSERT_EQ(kWidth, _img.Width());
  ASSERT_EQ(kHeight, _img.Height());
  ASSERT_EQ(kBpp_RGB, _img.BPP());
  ASSERT_EQ(kPitch_RGB, _img.Pitch());
  ASSERT_EQ(common::Image::PixelFormatType::RGB_INT8, _img.PixelFormat());
}

/////////////////////////////////////////////////
void CheckImageRGBA(const common::Image &_img)
{
  ASSERT_TRUE(_img.Valid());
  ASSERT_EQ(kWidth, _img.Width());
  ASSERT_EQ(kHeight, _img.Height());
  ASSERT_EQ(kBpp_RGBA, _img.BPP());
  ASSERT_EQ(kPitch_RGBA, _img.Pitch());
  ASSERT_EQ(common::Image::PixelFormatType::RGBA_INT8, _img.PixelFormat());
}

/////////////////////////////////////////////////
TEST_F(ImageTest, InvalidImage)
{
  common::Image img;
  ASSERT_EQ(-1, img.Load("/file/shouldn/never/exist.png"));
}

/////////////////////////////////////////////////
TEST_F(ImageTest, ImageProperties)
{
  common::Image img;
  ASSERT_EQ(-1, img.Load("/file/shouldn/never/exist.png"));

  // load image and test colors
  ASSERT_EQ(0, img.Load(kTestData));

  CheckImageRGBA(img);

  ASSERT_EQ(img.Pixel(0, 0), math::Color::Red);
  ASSERT_EQ(img.Pixel(85, 0), math::Color::Blue);
  ASSERT_EQ(kAvgColor, img.AvgColor());
  ASSERT_EQ(kMaxColor, img.MaxColor());

  ASSERT_TRUE(img.Filename().find("red_blue_colors.png") !=
      std::string::npos);
}

/////////////////////////////////////////////////
TEST_F(ImageTest, RGBData)
{
  // load image and test colors
  common::Image img;
  ASSERT_EQ(0, img.Load(kTestData));
  ASSERT_TRUE(img.Valid());

  CheckImageRGBA(img);

  // Check RGB data
  unsigned char *data = nullptr;
  unsigned int size = 0;
  img.RGBData(&data, size);
  ASSERT_EQ(kSize_RGB, size);
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

      ASSERT_EQ(0u, g) << i << "  " << j;
      if (j / channels < 80)
      {
        ASSERT_EQ(255u, r) << i << "  " << j / channels;
        ASSERT_EQ(0u, b) << i << "  " << j / channels;
      }
      else
      {
        ASSERT_EQ(0u, r) << i << "  " << j / channels;
        ASSERT_EQ(255u, b) << i << "  " << j / channels;
      }
    }
  }
}

/////////////////////////////////////////////////
TEST_F(ImageTest, Data)
{
  // load image and test colors
  common::Image img;
  ASSERT_EQ(0, img.Load(kTestData));
  ASSERT_TRUE(img.Valid());

  CheckImageRGBA(img);

  // Check RGBA data
  unsigned char *data = nullptr;
  unsigned int size = 0;
  img.Data(&data, size);
  ASSERT_EQ(kSize_RGBA, size);
  ASSERT_NE(nullptr, data);

  auto channels = 4u;
  auto step = img.Width() * channels;
  for (auto i = 0u; i < img.Height(); ++i)
  {
    for (auto j = 0u; j < step; j += channels)
    {
      unsigned int idx = i * step + j;
      unsigned int r = data[idx];
      unsigned int g = data[idx+1];
      unsigned int b = data[idx+2];
      unsigned int a = data[idx+3];

      ASSERT_EQ(0u, g) << i << "  " << j;
      ASSERT_EQ(255u, a) << i << "  " << j;
      if (j / channels < 80)
      {
        ASSERT_EQ(255u, r) << i << "  " << j / channels;
        ASSERT_EQ(0u, b) << i << "  " << j / channels;
      }
      else
      {
        ASSERT_EQ(0u, r) << i << "  " << j / channels;
        ASSERT_EQ(255u, b) << i << "  " << j / channels;
      }
    }
  }
}

/////////////////////////////////////////////////
TEST_F(ImageTest, SetFromData)
{
  // load image and test colors
  common::Image img;
  ASSERT_EQ(0, img.Load(kTestData));
  ASSERT_TRUE(img.Valid());

  unsigned char *data = nullptr;
  unsigned int size = 0;
  img.Data(&data, size);
  ASSERT_EQ(39204u, size);
  ASSERT_NE(nullptr, data);


  common::Image img2;
  img2.SetFromData(data, img.Width(), img.Height(), img.PixelFormat());
  ASSERT_TRUE(img2.Valid());
  ASSERT_EQ(common::Image::PixelFormatType::RGBA_INT8, img2.PixelFormat());
  ASSERT_EQ(121u, img2.Width());
  ASSERT_EQ(81u, img2.Height());
  ASSERT_EQ(32u, img2.BPP());
  ASSERT_EQ(484, img2.Pitch());
  ASSERT_EQ(img2.Pixel(0, 0), math::Color::Red);
  ASSERT_EQ(img2.Pixel(85, 0), math::Color::Blue);
  ASSERT_EQ(img2.AvgColor(), math::Color(0.661157f, 0, 0.338843f, 1));
  ASSERT_EQ(img2.MaxColor(), math::Color::Red);
}

/*
  // save image then reload and test colors
  std::string testSaveImage =
    common::testing::TempPath("test_red_blue_save.png");
  img.SavePNG(testSaveImage);

  common::Image img2;
  img2.Load(testSaveImage);
  ASSERT_TRUE(img2.Valid());
  ASSERT_EQ(common::Image::PixelFormatType::RGB_INT8, img2.PixelFormat());
  ASSERT_EQ(121u, img2.Width());
  ASSERT_EQ(81u, img2.Height());
  ASSERT_EQ(24u, img2.BPP());
  ASSERT_EQ(363, img2.Pitch());
  ASSERT_EQ(img2.Pixel(0, 0), math::Color::Red);
  ASSERT_EQ(img2.Pixel(85, 0), math::Color::Blue);
  ASSERT_EQ(img2.AvgColor(), math::Color(0.661157f, 0, 0.338843f, 1));
  ASSERT_EQ(img2.MaxColor(), math::Color::Red);

  // Check data
  data = nullptr;
  size = 0;
  img2.Data(&data, size);
  ASSERT_EQ(29403u, size);
  ASSERT_NE(nullptr, data);

  channels = 3u;
  step = img2.Width() * channels;
  for (auto i = 0u; i < img2.Height(); ++i)
  {
    for (auto j = 0u; j < step; j += channels)
    {
      unsigned int idx = i * step + j;
      unsigned int r = data[idx];
      unsigned int g = data[idx+1];
      unsigned int b = data[idx+2];

      ASSERT_EQ(0u, g) << i << "  " << j;
      if (j / channels < 80)
      {
        ASSERT_EQ(255u, r) << i << "  " << j / channels;
        ASSERT_EQ(0u, b) << i << "  " << j / channels;
      }
      else
      {
        ASSERT_EQ(0u, r) << i << "  " << j / channels;
        ASSERT_EQ(255u, b) << i << "  " << j / channels;
      }
    }
  }

  common::Image img3;
  img3.SetFromData(data, img3.Width(), img3.Height(), img3.PixelFormat());
  ASSERT_TRUE(img3.Valid());

  ASSERT_EQ(common::Image::PixelFormatType::RGB_INT8, img3.PixelFormat());
  ASSERT_EQ(121u, img3.Width());
  ASSERT_EQ(81u, img3.Height());
  ASSERT_EQ(24u, img3.BPP());
  ASSERT_EQ(363, img3.Pitch());
  ASSERT_EQ(img3.Pixel(0, 0), math::Color::Red);
  ASSERT_EQ(img3.Pixel(85, 0), math::Color::Blue);
  ASSERT_EQ(img3.AvgColor(), math::Color(0.661157f, 0, 0.338843f, 1));
  ASSERT_EQ(img3.MaxColor(), math::Color::Red);
}
*/

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
