/*
 * Copyright (C) 2016 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include "ignition/common/Console.hh"
#include "ignition/common/ImageHeightmap.hh"

using namespace ignition;
using namespace common;

//////////////////////////////////////////////////
ImageHeightmap::ImageHeightmap()
{
}

//////////////////////////////////////////////////
int ImageHeightmap::Load(const std::string &_filename)
{
  if (this->img.Load(_filename) != 0)
  {
    ignerr << "Unable to load image file as a terrain [" << _filename << "]\n";
    return -1;
  }

  return 0;
}

//////////////////////////////////////////////////
void ImageHeightmap::FillHeightMap(int _subSampling,
    unsigned int _vertSize, const ignition::math::Vector3d &_size,
    const ignition::math::Vector3d &_scale, bool _flipY,
    std::vector<float> &_heights)
{
  // Resize the vector to match the size of the vertices.
  _heights.resize(_vertSize * _vertSize);

  int imgHeight = this->Height();
  int imgWidth = this->Width();

  IGN_ASSERT(imgWidth == imgHeight, "Heightmap image must be square");

  // Bytes per row
  unsigned int pitch = this->img.Pitch();

  // Bytes per pixel
  unsigned int bpp = pitch / imgWidth;

  unsigned char *data = nullptr;
  unsigned int count;
  this->img.Data(&data, count);

  // Get the image format so we can arrange our heightmap
  // Currently supported: 8-bit and 16-bit.
  auto imgFormat = this->img.PixelFormat();

  double maxPixelValue;
  if(imgFormat == ignition::common::Image::PixelFormatType::L_INT8 ||
    imgFormat == ignition::common::Image::PixelFormatType::RGB_INT8 ||
    imgFormat == ignition::common::Image::PixelFormatType::RGBA_INT8 ||
    imgFormat == ignition::common::Image::PixelFormatType::BAYER_BGGR8 ||
    imgFormat == ignition::common::Image::PixelFormatType::BAYER_GBRG8 ||
    imgFormat == ignition::common::Image::PixelFormatType::BAYER_GRBG8 ||
    imgFormat == ignition::common::Image::PixelFormatType::BAYER_GRBG8 ||
    imgFormat == ignition::common::Image::PixelFormatType::BAYER_RGGB8 ||
    imgFormat == ignition::common::Image::PixelFormatType::BGR_INT8 ||
    imgFormat == ignition::common::Image::PixelFormatType::BGRA_INT8)
  {
    maxPixelValue = 255.0;
    // Iterate over all the vertices
    for (unsigned int y = 0; y < _vertSize; ++y)
    {
      // yf ranges between 0 and 4
      double yf = y / static_cast<double>(_subSampling);
      int y1 = static_cast<int>(std::floor(yf));
      int y2 = static_cast<int>(std::ceil(yf));
      if (y2 >= imgHeight)
        y2 = imgHeight-1;
      double dy = yf - y1;

      for (unsigned int x = 0; x < _vertSize; ++x)
      {
        double xf = x / static_cast<double>(_subSampling);
        int x1 = static_cast<int>(std::floor(xf));
        int x2 = static_cast<int>(std::ceil(xf));
        if (x2 >= imgWidth)
          x2 = imgWidth-1;
        double dx = xf - x1;

        double px1 = static_cast<int>(
          data[y1 * pitch / bpp + x1]) / maxPixelValue;
        double px2 = static_cast<int>(
          data[y1 * pitch / bpp + x2]) / maxPixelValue;
        float h1 = (px1 - ((px1 - px2) * dx));

        double px3 = static_cast<int>(
          data[y2 * pitch / bpp + x1]) / maxPixelValue;
        double px4 = static_cast<int>(
          data[y2 * pitch / bpp + x2]) / maxPixelValue;
        float h2 = (px3 - ((px3 - px4) * dx));

        float h = (h1 - ((h1 - h2) * dy)) * _scale.Z();

        // invert pixel definition so 1=ground, 0=full height,
        //   if the terrain size has a negative z component
        //   this is mainly for backward compatibility
        if (_size.Z() < 0)
          h = 1.0 - h;

        // Store the height for future use
        if (!_flipY)
          _heights[y * _vertSize + x] = h;
        else
          _heights[(_vertSize - y - 1) * _vertSize + x] = h;
      }
    }
    delete [] data;
  }
  else if(imgFormat == ignition::common::Image::PixelFormatType::BGR_INT16 ||
    imgFormat == ignition::common::Image::PixelFormatType::L_INT16 ||
    imgFormat == ignition::common::Image::PixelFormatType::RGB_FLOAT16 ||
    imgFormat == ignition::common::Image::PixelFormatType::RGB_INT16 ||
    imgFormat == ignition::common::Image::PixelFormatType::R_FLOAT16)
  {
    maxPixelValue = 65535.0;
    uint16_t *dataShort = reinterpret_cast<uint16_t *>(data);
    // Iterate over all the vertices
    for (unsigned int y = 0; y < _vertSize; ++y)
    {
      // yf ranges between 0 and 4
      double yf = y / static_cast<double>(_subSampling);
      int y1 = static_cast<int>(std::floor(yf));
      int y2 = static_cast<int>(std::ceil(yf));
      if (y2 >= imgHeight)
        y2 = imgHeight-1;
      double dy = yf - y1;

      for (unsigned int x = 0; x < _vertSize; ++x)
      {
        double xf = x / static_cast<double>(_subSampling);
        int x1 = static_cast<int>(std::floor(xf));
        int x2 = static_cast<int>(std::ceil(xf));
        if (x2 >= imgWidth)
          x2 = imgWidth-1;
        double dx = xf - x1;

        double px1 = static_cast<int>(
          dataShort[y1 * pitch / bpp + x1]) / maxPixelValue;
        double px2 = static_cast<int>(
          dataShort[y1 * pitch / bpp + x2]) / maxPixelValue;
        float h1 = (px1 - ((px1 - px2) * dx));

        double px3 = static_cast<int>(
          dataShort[y2 * pitch / bpp + x1]) / maxPixelValue;
        double px4 = static_cast<int>(
          dataShort[y2 * pitch / bpp + x2]) / maxPixelValue;
        float h2 = (px3 - ((px3 - px4) * dx));

        float h = (h1 - ((h1 - h2) * dy)) * _scale.Z();

        // invert pixel definition so 1=ground, 0=full height,
        //   if the terrain size has a negative z component
        //   this is mainly for backward compatibility
        if (_size.Z() < 0)
          h = 1.0 - h;

        // Store the height for future use
        if (!_flipY)
          _heights[y * _vertSize + x] = h;
        else
          _heights[(_vertSize - y - 1) * _vertSize + x] = h;
      }
    }
    delete [] dataShort;
  }
  else
  {
    ignerr << "Unknown image format, heightmap will not be loaded" << std::endl;
    return;
  }

}

//////////////////////////////////////////////////
std::string ImageHeightmap::Filename() const
{
  return this->img.Filename();
}

//////////////////////////////////////////////////
unsigned int ImageHeightmap::Height() const
{
  return this->img.Height();
}

//////////////////////////////////////////////////
unsigned int ImageHeightmap::Width() const
{
  return this->img.Width();
}

//////////////////////////////////////////////////
float ImageHeightmap::MaxElevation() const
{
  return this->img.MaxColor().R();
}
