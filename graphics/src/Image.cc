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
#include <algorithm>
#define STB_IMAGE_IMPLEMENTATION
#include "STB/stb_image.h"

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
#if defined(__APPLE__)
// Suppress deprecation warning around snprintf
// Ref: https://github.com/nothings/stb/issues/1446
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "STB/stb_image_write.h"
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#if defined(__APPLE__)
#pragma clang diagnostic pop
#endif

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "STB/stb_image_resize2.h"
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

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
      /// \brief bitmap data
      public: void *bitmap{nullptr};

      /// \brief path name of the image file
      public: std::string fullName;

      /// \brief Width of the image
      public: int width;

      /// \brief Height of the image
      public: int height;

      /// \brief the number of channels per pixel
      ///
      ///     channels    components
      ///       1           grey
      ///       2           grey, alpha
      ///       3           red, green, blue
      ///       4           red, green, blue, alpha
      public: int channels;

      /// \brief the number of bits per pixel
      public: int bits_per_channel;

      /// \brief Converts bitmap data to the given number of channels
      public: std::vector<unsigned char> DataWithChannels(int out_channels) const;

      /// \brief Implementation of Data, returns vector of bytes
      public: std::vector<unsigned char> DataImpl(void *_img, size_t size) const;
    };
  }  // namespace common
}  // namespace gz

//////////////////////////////////////////////////
Image::Image(const std::string &_filename)
: dataPtr(gz::utils::MakeImpl<Implementation>())
{
  this->dataPtr->bitmap = NULL;
  if (!_filename.empty())
  {
    std::string filename = gz::common::findFile(_filename);
    if (!filename.empty())
      this->Load(filename);
    else
      gzerr << "Unable to find image[" << _filename << "]\n";
  }
}

//////////////////////////////////////////////////
Image::~Image()
{
  if (this->dataPtr->bitmap)
    stbi_image_free(this->dataPtr->bitmap);
  this->dataPtr->bitmap = NULL;
}


//////////////////////////////////////////////////
int Image::Load(const std::string &_filename)
{
  this->dataPtr->fullName = _filename;
  if (!exists(this->dataPtr->fullName))
  {
    this->dataPtr->fullName = common::findFile(_filename);
  }

  if (exists(this->dataPtr->fullName))
  {
    if (this->dataPtr->bitmap)
      stbi_image_free(this->dataPtr->bitmap);
    this->dataPtr->bitmap = NULL;

    void *bitmap = NULL;
    int bpc = 0;
    const char *fn = this->dataPtr->fullName.c_str();
    int w, h, n;

    if (stbi_is_hdr(fn))
    {
      bitmap = stbi_loadf(fn, &w, &h, &n, 0);
      bpc = 32;
    }
    else if (stbi_is_16_bit(fn))
    {
      bitmap = stbi_load_16(fn, &w, &h, &n, 0);
      bpc = 16;
    }
    else
    {
      bitmap = stbi_load(fn, &w, &h, &n, 0);
      bpc = 8;
    }

    if (bitmap == NULL)
    {
      gzerr << "Failed to load file [" << this->dataPtr->fullName
            << "]: " << stbi_failure_reason() << std::endl;
      return -1;
    }

    this->dataPtr->bitmap = bitmap;
    this->dataPtr->bits_per_channel = bpc;
    this->dataPtr->width = w;
    this->dataPtr->height = h;
    this->dataPtr->channels = n;

    return 0;
  }

  gzerr << "Unable to open image file[" << this->dataPtr->fullName
        << "], check your GZ_RESOURCE_PATH settings.\n";
  return -1;
}

//////////////////////////////////////////////////
void Image::SavePNG(const std::string &_filename)
{
  if (this->dataPtr->bits_per_channel != 8)
  {
    gzerr << "Cannot write " << this->dataPtr->bits_per_channel
          << "-bit PNG image (" << _filename << ")\n";
    return;
  }
  int ret = stbi_write_png(_filename.c_str(), dataPtr->width, dataPtr->height,
                           dataPtr->channels, dataPtr->bitmap,
                           dataPtr->width * dataPtr->channels);
  if (ret == 0)
    gzerr << "Error writing PNG image to [" << _filename << "]\n";
}

//////////////////////////////////////////////////
static void vectorWriteFunc(void *_context, void *_data, int _size)
{
  auto buffer = reinterpret_cast<std::vector<unsigned char> *>(_context);
  auto prev_size = buffer->size();
  buffer->resize(prev_size + _size);
  std::memcpy(buffer->data() + prev_size, _data, _size);
}

void Image::SavePNGToBuffer(std::vector<unsigned char>& buffer)
{
  if (this->dataPtr->bits_per_channel != 8)
  {
    gzerr << "Cannot export " << this->dataPtr->bits_per_channel
          << "-bit PNG image\n";
    return;
  }
  int ret = stbi_write_png_to_func(
      vectorWriteFunc, &buffer, dataPtr->width, dataPtr->height,
      dataPtr->channels, dataPtr->bitmap,
      dataPtr->width * dataPtr->channels * dataPtr->bits_per_channel / 8);
  if (ret == 0)
    gzerr << "Error exporting PNG image\n";
}

//////////////////////////////////////////////////
void Image::SetFromData(const unsigned char *_data,
    unsigned int _width,
    unsigned int _height,
    Image::PixelFormatType _format)
{
  if (this->dataPtr->bitmap)
    stbi_image_free(this->dataPtr->bitmap);
  this->dataPtr->bitmap = NULL;

  unsigned int bpp;
  int scanlineBytes;

  if (_format == L_INT8)
  {
    bpp = 8;
    scanlineBytes = _width;
  }
  else if (_format == RGB_INT8)
  {
    bpp = 24;
    scanlineBytes = _width * 3;
  }
  else if (_format == RGBA_INT8)
  {
    bpp = 32;
    scanlineBytes = _width * 4;
  }
  else
  {
    gzerr << "Unable to handle format[" << _format << "]\n";
    return;
  }

  const size_t size = _height * scanlineBytes;
  this->dataPtr->bitmap = STBI_MALLOC(size);
  if (this->dataPtr->bitmap == NULL)
  {
    gzerr << "Error allocating image memory\n";
    return;
  }
  memcpy(this->dataPtr->bitmap, _data, size);

  this->dataPtr->width = _width;
  this->dataPtr->height = _height;
  this->dataPtr->channels = bpp / 8;
  this->dataPtr->bits_per_channel = 8;
}

//////////////////////////////////////////////////
void Image::SetFromCompressedData(unsigned char *_data,
                                  unsigned int _size,
                                  Image::PixelFormatType _format)
{
  if (this->dataPtr->bitmap)
    stbi_image_free(this->dataPtr->bitmap);
  this->dataPtr->bitmap = NULL;

  void *bitmap = NULL;
  int bpc = 0;
  int w, h, n;

  switch (_format)
  {
  case COMPRESSED_PNG:  // fall through
  case COMPRESSED_JPEG:
    if (stbi_is_hdr_from_memory(_data, _size))
    {
      bitmap = stbi_loadf_from_memory(_data, _size, &w, &h, &n, 0);
      bpc = 32;
    }
    else if (stbi_is_16_bit_from_memory(_data, _size))
    {
      bitmap = stbi_load_16_from_memory(_data, _size, &w, &h, &n, 0);
      bpc = 16;
    }
    else
    {
      bitmap = stbi_load_from_memory(_data, _size, &w, &h, &n, 0);
      bpc = 8;
    }
    break;
  default:
    gzerr << "Unable to handle format[" << _format << "]\n";
    return;
  }
  this->dataPtr->bitmap = bitmap;
  this->dataPtr->bits_per_channel = bpc;
  this->dataPtr->width = w;
  this->dataPtr->height = h;
  this->dataPtr->channels = n;
}

//////////////////////////////////////////////////
int Image::Pitch() const
{
  return this->dataPtr->width * this->dataPtr->channels *
         this->dataPtr->bits_per_channel / 8;
}

//////////////////////////////////////////////////
std::vector<unsigned char>
Image::Implementation::DataWithChannels(int out_channels) const
{
  std::vector<unsigned char> data;
  const size_t size =
      this->width * this->height * this->channels * this->bits_per_channel / 8;

  if (this->channels != out_channels)
  {
    // Copy data because stbi__convert_format() frees the original data
    unsigned char *bitmap_copy = (unsigned char *)STBI_MALLOC(size);
    if (bitmap_copy == NULL)
    {
      gzerr << "Error allocating image memory\n";
      return std::vector<unsigned char>();
    }
    memcpy(bitmap_copy, this->bitmap, size);

    unsigned char *bitmap_rgb = NULL;
    switch (this->bits_per_channel)
    {
    case 8:
      bitmap_rgb = stbi__convert_format(
          bitmap_copy, this->channels, out_channels, this->width, this->height);
      break;
    case 16:
      bitmap_rgb = reinterpret_cast<unsigned char *>(stbi__convert_format16(
          reinterpret_cast<uint16_t *>(bitmap_copy), this->channels,
          out_channels, this->width, this->height));
      break;
    case 32:  // not implemented in stbi
      break;
    }
    if (bitmap_rgb == NULL)
    {
      gzerr << "Error converting image to " << out_channels << " channels\n";
      return std::vector<unsigned char>();
    }
    data =
        this->DataImpl(bitmap_rgb, this->width * this->height * out_channels *
                                       this->bits_per_channel / 8);
    STBI_FREE(bitmap_rgb);
  }
  else
  {
    data = this->DataImpl(this->bitmap, size);
  }
  return data;
}

//////////////////////////////////////////////////
std::vector<unsigned char> Image::RGBData() const
{
  return this->dataPtr->DataWithChannels(3);
}

//////////////////////////////////////////////////
std::vector<unsigned char> Image::RGBAData() const
{
  return this->dataPtr->DataWithChannels(4);
}

//////////////////////////////////////////////////
std::vector<unsigned char> Image::Data() const
{
  std::vector<unsigned char> data;
  const size_t size = this->dataPtr->height * this->Pitch();
  data = this->dataPtr->DataImpl(this->dataPtr->bitmap, size);
  return data;
}

//////////////////////////////////////////////////
std::vector<unsigned char> Image::Implementation::DataImpl(void *_img, size_t _size) const
{
  std::vector<unsigned char> data(_size);
  memcpy(data.data(), _img, _size);
  return data;
}

//////////////////////////////////////////////////
unsigned int Image::Width() const
{
  if (!this->Valid())
    return 0;

  return this->dataPtr->width;
}

//////////////////////////////////////////////////
unsigned int Image::Height() const
{
  if (!this->Valid())
    return 0;

  return this->dataPtr->height;
}

//////////////////////////////////////////////////
unsigned int Image::BPP() const
{
  if (!this->Valid())
    return 0;

  return this->dataPtr->channels * this->dataPtr->bits_per_channel;
}

//////////////////////////////////////////////////
template <typename T, unsigned DIV>
static void readPixel(math::Color &clr, const void *pixel_ptr, int channels)
{
  auto pixel = reinterpret_cast<const T *>(pixel_ptr);
  float div = static_cast<float>(DIV);

  switch (channels)
  {
  case 1: /* Grayscale */
    clr.Set(pixel[0] / div, pixel[0] / div, pixel[0] / div);
    break;
  case 3: /* RGB */
    clr.Set(pixel[0] / div, pixel[1] / div, pixel[2] / div);
    break;
  case 4: /* RGBA */
    clr.Set(pixel[0] / div, pixel[1] / div, pixel[2] / div, pixel[3] / div);
    break;
  default:
    gzerr << "Image: Unsupported number of channels [" << channels << "] \n";
  }
}

math::Color Image::Pixel(unsigned int _x, unsigned int _y) const
{
  math::Color clr;

  if (!this->Valid())
    return clr;

  if (_x >= this->Width() || _y >= this->Height())
  {
    gzerr << "Image: Coordinates out of range["
      << _x << ", " << _y << "] \n";
    return clr;
  }

  // FreeImage used to index rows in the opposite way
  _y = this->dataPtr->height - 1 - _y;

  void *pixel_ptr = (unsigned char *)this->dataPtr->bitmap +
                    _y * this->dataPtr->width * this->BPP() / 8 +
                    _x * this->BPP() / 8;

  switch (this->dataPtr->bits_per_channel)
  {
  case 8:
    readPixel<unsigned char, 255>(clr, pixel_ptr, this->dataPtr->channels);
    break;
  case 16:
    readPixel<uint16_t, 65535>(clr, pixel_ptr, this->dataPtr->channels);
    break;
  case 32:
    readPixel<float, 1>(clr, pixel_ptr, this->dataPtr->channels);
    break;
  default:
    gzerr << "Image: Unsupported bits per channel ["
          << this->dataPtr->bits_per_channel << "] \n";
    return clr;
  }

  return clr;
}

//////////////////////////////////////////////////
math::Color Image::AvgColor() const
{
  unsigned int x, y;
  double rsum, gsum, bsum;
  math::Color pixel;

  rsum = gsum = bsum = 0.0;
  for (y = 0; y < this->Height(); ++y)
  {
    for (x = 0; x < this->Width(); ++x)
    {
      pixel = this->Pixel(x, y);
      rsum += pixel.R();
      gsum += pixel.G();
      bsum += pixel.B();
    }
  }

  rsum /= (this->Width() * this->Height());
  gsum /= (this->Width() * this->Height());
  bsum /= (this->Width() * this->Height());

  return math::Color(rsum, gsum, bsum);
}

//////////////////////////////////////////////////
math::Color Image::MaxColor() const
{
  math::Color maxClr;

  if (!this->Valid())
    return maxClr;

  maxClr.Set(0, 0, 0, 0);
  for (unsigned int y = 0; y < this->Height(); y++)
  {
    for (unsigned int x = 0; x < this->Width(); x++)
    {
      math::Color clr = this->Pixel(x, y);
      if (clr.R() + clr.G() + clr.B() > maxClr.R() + maxClr.G() + maxClr.B())
      {
        maxClr = clr;
      }
    }
  }

  return maxClr;
}

//////////////////////////////////////////////////
void Image::Rescale(int _width, int _height)
{
  stbir_pixel_layout pixel_layout;
  stbir_datatype data_type;

  switch (this->dataPtr->channels)
  {
  case 1:
    pixel_layout = STBIR_1CHANNEL;
    break;
  case 3:
    pixel_layout = STBIR_RGB;
    break;
  case 4:
    pixel_layout = STBIR_RGBA;
    break;
  default:
    gzerr << "Cannot rescale " << dataPtr->channels << "-channel image\n";
    stbi_image_free(dataPtr->bitmap);
    dataPtr->bitmap = NULL;
    return;
  }

  switch (this->dataPtr->bits_per_channel)
  {
  case 8:
    data_type = STBIR_TYPE_UINT8;
    break;
  case 16:
    data_type = STBIR_TYPE_UINT16;
    break;
  case 32:
    data_type = STBIR_TYPE_FLOAT;
    break;
  default:
    gzerr << "Cannot rescale " << dataPtr->bits_per_channel << "-bit image\n";
    stbi_image_free(dataPtr->bitmap);
    dataPtr->bitmap = NULL;
    return;
  }

  void *ret = stbir_resize(dataPtr->bitmap, dataPtr->width, dataPtr->height, 0,
                           NULL, _width, _height, 0, pixel_layout, data_type,
                           STBIR_EDGE_CLAMP, STBIR_FILTER_DEFAULT);

  stbi_image_free(dataPtr->bitmap);
  dataPtr->bitmap = NULL;

  if (ret != NULL)
  {
    this->dataPtr->bitmap = ret;
    this->dataPtr->width = _width;
    this->dataPtr->height = _height;
  }
  else
  {
    gzerr << "Rescaling image failed\n";
  }
}

//////////////////////////////////////////////////
bool Image::Valid() const
{
  return this->dataPtr->bitmap != NULL;
}

//////////////////////////////////////////////////
std::string Image::Filename() const
{
  return this->dataPtr->fullName;
}

//////////////////////////////////////////////////
Image::PixelFormatType Image::PixelFormat() const
{
  Image::PixelFormatType types_8b[] = {
      UNKNOWN_PIXEL_FORMAT, L_INT8, UNKNOWN_PIXEL_FORMAT, RGB_INT8, RGBA_INT8};
  Image::PixelFormatType types_16b[] = {UNKNOWN_PIXEL_FORMAT, L_INT16,
                                        UNKNOWN_PIXEL_FORMAT, RGB_INT16,
                                        RGBA_INT16};
  Image::PixelFormatType types_32b[] = {UNKNOWN_PIXEL_FORMAT, R_FLOAT32,
                                        UNKNOWN_PIXEL_FORMAT, RGB_FLOAT32,
                                        UNKNOWN_PIXEL_FORMAT};
  Image::PixelFormatType *types;

  switch (this->dataPtr->bits_per_channel)
  {
  case 8:
    types = types_8b;
    break;
  case 16:
    types = types_16b;
    break;
  case 32:
    types = types_32b;
    break;
  default:
    return UNKNOWN_PIXEL_FORMAT;
  }
  if (this->dataPtr->channels >= 0 && this->dataPtr->channels <= 4)
  {
    return types[this->dataPtr->channels];
  }
  else
  {
    return UNKNOWN_PIXEL_FORMAT;
  }
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

//////////////////////////////////////////////////
std::vector<unsigned char> Image::ChannelData(Channel _channel) const
{
  const int bpc = this->dataPtr->bits_per_channel;
  const int ch = this->dataPtr->channels;
  const void *bitmap = this->dataPtr->bitmap;

  if ((ch == 1 && _channel != Channel::RED) ||
      (ch == 3 && _channel == Channel::ALPHA))
  {
    gzerr << "Failed to extract channel data for input channel: "
          << static_cast<int>(_channel) << std::endl;
    return {};
  }
  std::vector<unsigned char> data;
  data.resize(Width() * Height());

  int ch_i = static_cast<int>(_channel);

  for (size_t i = 0; i < Width() * Height(); i++)
  {
    switch (bpc)
    {
    case 8: {
      auto * pixel = reinterpret_cast<const uint8_t *>(bitmap) + i * ch;
      data[i] = pixel[ch_i];
      break;
    }
    case 16: {
      auto * pixel = reinterpret_cast<const uint16_t *>(bitmap) + i * ch;
      data[i] = pixel[ch_i] >> 8;
      break;
    }
    case 32: {
      auto * pixel = reinterpret_cast<const float *>(bitmap) + i * ch;
      data[i] = static_cast<unsigned char>(pixel[ch_i] * 255.0f);
      break;
    }
    }
  }
  return data;
}
