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

#include "ImageLoader.hh"
#include "ImageLoaderSTB.hh"
#include "ImageLoaderFreeImage.hh"

#include <cstring>
#include <string>

#include <gz/common/Console.hh>
#include <gz/common/Util.hh>
#include <gz/common/Image.hh>

using namespace gz;
using namespace common;

namespace gz
{
  namespace common
  {
    /// \brief Private data class
    class Image::Implementation
    {
      public: std::shared_ptr<ImageLoader> loader;
      public: std::shared_ptr<ImageData> imageData;
    };
  }
}

//////////////////////////////////////////////////
Image::Image(const std::string &_filename)
: dataPtr(gz::utils::MakeImpl<Implementation>())
{
  this->dataPtr->loader = std::make_unique<ImageLoaderFreeImage>();

  if (!_filename.empty())
  {
    this->Load(_filename);
  }
}

//////////////////////////////////////////////////
Image::~Image() = default;

//////////////////////////////////////////////////
int Image::Load(const std::string &_filename)
{
  this->dataPtr->imageData = this->dataPtr->loader->Load(_filename);

  if (this->dataPtr->imageData != nullptr)
  {
    std::cout << "Loaded image: " << _filename << " ("
      << this->dataPtr->imageData->width << "x" << this->dataPtr->imageData->height << ")" << std::endl;
    std::cout << this->dataPtr->imageData->data.size() << std::endl;
  }

  return this->dataPtr->imageData == nullptr ? -1 : 0;
}

//////////////////////////////////////////////////
void Image::SavePNG(const std::string &_filename)
{
}

//////////////////////////////////////////////////
void Image::SavePNGToBuffer(std::vector<unsigned char>& buffer)
{
}

//////////////////////////////////////////////////
void Image::SetFromData(const unsigned char *_data,
    unsigned int _width,
    unsigned int _height,
    Image::PixelFormatType _format)
{
}

//////////////////////////////////////////////////
void Image::SetFromCompressedData(unsigned char *_data,
                                  unsigned int _size,
                                  Image::PixelFormatType _format)
{
}

//////////////////////////////////////////////////
int Image::Pitch() const
{
  if (!this->Valid())
    return {};

  return this->dataPtr->imageData->pitch;
}

//////////////////////////////////////////////////
std::vector<unsigned char> Image::RGBData() const
{
  if (!this->Valid())
    return {};

  if (this->dataPtr->imageData->pixel_format == Image::RGBA_INT8)
  {
    std::vector<unsigned char> ret;

    auto rgba_data = this->dataPtr->imageData->data;

    ret.reserve(rgba_data.size() * 3 / 4);
  }
}

//////////////////////////////////////////////////
std::vector<unsigned char> Image::RGBAData() const
{
  if (!this->Valid())
    return {};

  return this->dataPtr->imageData->data;
}

//////////////////////////////////////////////////
std::vector<unsigned char> Image::Data() const
{
  if (!this->Valid())
    return {};

  return this->dataPtr->imageData->data;
}

//////////////////////////////////////////////////
unsigned int Image::Width() const
{
  if (!this->Valid())
    return 0;

  return this->dataPtr->imageData->width;
}

//////////////////////////////////////////////////
unsigned int Image::Height() const
{
  if (!this->Valid())
    return 0;

  return this->dataPtr->imageData->height;
}

//////////////////////////////////////////////////
unsigned int Image::BPP() const
{
  return this->BitsPerPixel();
}

//////////////////////////////////////////////////
unsigned int Image::BitsPerPixel() const
{
  if (!this->Valid())
    return 0;

  return this->dataPtr->imageData->bpp * 8;
}

//////////////////////////////////////////////////
unsigned int Image::BytesPerPixel() const
{
  if (!this->Valid())
    return 0;

  return this->dataPtr->imageData->bpp;
}


//////////////////////////////////////////////////
math::Color Image::Pixel(unsigned int _x, unsigned int _y) const
{
  if (!this->Valid())
    return {};

  if (this->dataPtr->imageData->pixel_format == gz::common::Image::RGBA_INT8)
  {
    if (!this->dataPtr->imageData->InBounds(_x, _y))
    {
      gzerr << "Pixel coordinates [" << _x << "x" << _y << "] are out of the image bounds" << std::endl;
      return {};
    }

    auto pixel = this->dataPtr->imageData->At(_x, _y);
    auto r = *pixel++;
    auto g = *pixel++;
    auto b = *pixel++;
    return math::Color(r, g, b);
  }

  return {};
}

//////////////////////////////////////////////////
math::Color Image::AvgColor() const
{
  return {};
}

//////////////////////////////////////////////////
math::Color Image::MaxColor() const
{
  return {};
}

//////////////////////////////////////////////////
void Image::Rescale(int _width, int _height)
{
}

//////////////////////////////////////////////////
bool Image::Valid() const
{
  return this->dataPtr->imageData != nullptr;
}

//////////////////////////////////////////////////
std::string Image::Filename() const
{
  if (!this->Valid())
    return "";
  return this->dataPtr->imageData->filename;
}

//////////////////////////////////////////////////
Image::PixelFormatType Image::PixelFormat() const
{
  return this->dataPtr->imageData->pixel_format;
}

/////////////////////////////////////////////////
Image::PixelFormatType Image::ConvertPixelFormat(const std::string &_format)
{
  // Handle old format strings
  if (_format == "L8")
    return L_INT8;
  else if (_format == "R8G8B8")
    return RGB_INT8;

  for (unsigned int i = 0; i < PIXEL_FORMAT_COUNT; ++i)
    if (PixelFormatNames[i] == _format)
      return static_cast<PixelFormatType>(i);

  return UNKNOWN_PIXEL_FORMAT;
}
