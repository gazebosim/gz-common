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
#ifdef BOOL
#undef BOOL
#endif
#include <FreeImage.h>

#include <cstring>
#include <optional>
#include <string>

#include <gz/common/Console.hh>
#include <gz/common/Util.hh>
#include <gz/common/Image.hh>

using namespace ignition;
using namespace common;

namespace ignition
{
  namespace common
  {
    /// \brief Private data class
    class Image::Implementation
    {
      /// \brief bitmap data
      public: FIBITMAP *bitmap;

      /// \brief path name of the image file
      public: std::string fullName;

<<<<<<< HEAD
      /// \brief Implementation of GetData
      public: void DataImpl(unsigned char **_data, unsigned int &_count,
          FIBITMAP *_img) const;

      /// \brief Returns true if SwapRedBlue can and should be called
      /// If it returns false, it may not be safe to call SwapRedBlue
      /// (it could lead to memory corruption!). See CanSwapRedBlue
      /// \return True if we should call SwapRedBlue
      public: bool ShouldSwapRedBlue() const;

      /// \brief Returns true if SwapRedBlue is safe to be called
      /// \return False if it is NOT safe to call SwapRedBlue
      public: bool CanSwapRedBlue() const;

      /// \brief Swap red and blue pixels
      /// \param[in] _width Width of the image
      /// \param[in] _height Height of the image
      /// \return bitmap data with red and blue pixels swapped
      public: FIBITMAP* SwapRedBlue(const unsigned int &_width,
                                    const unsigned int &_height);
=======
      /// \brief Width of the image
      public: int width{0};

      /// \brief Height of the image
      public: int height{0};

      /// \brief the number of channels per pixel
      ///
      ///     channels    components
      ///       1           grey
      ///       2           grey, alpha
      ///       3           red, green, blue
      ///       4           red, green, blue, alpha
      public: int channels{0};

      /// \brief the number of bits per pixel
      public: int bits_per_channel{0};

      /// \brief Converts bitmap data to the given number of channels
      public: std::vector<unsigned char> DataWithChannels(int out_channels)
        const;

      /// \brief Implementation of Data, returns vector of bytes
      public: std::vector<unsigned char> DataImpl(void *_img, size_t size)
        const;
>>>>>>> b632fbd (Optimize texture image decoding and channel conversion (#817))
    };
  }
}

static int count = 0;

//////////////////////////////////////////////////
Image::Image(const std::string &_filename)
: dataPtr(ignition::utils::MakeImpl<Implementation>())
{
  if (count == 0)
    FreeImage_Initialise();

  count++;

  this->dataPtr->bitmap = NULL;
  if (!_filename.empty())
  {
    std::string filename = findFile(_filename);
    if (!filename.empty())
      this->Load(filename);
    else
      ignerr << "Unable to find image[" << _filename << "]\n";
  }
}

//////////////////////////////////////////////////
Image::~Image()
{
  count--;

  if (this->dataPtr->bitmap)
    FreeImage_Unload(this->dataPtr->bitmap);
  this->dataPtr->bitmap = NULL;

  if (count == 0)
    FreeImage_DeInitialise();
}


//////////////////////////////////////////////////
int Image::Load(const std::string &_filename)
{
  this->dataPtr->fullName = _filename;
  if (!exists(this->dataPtr->fullName))
  {
    this->dataPtr->fullName = findFile(_filename);
  }

  if (exists(this->dataPtr->fullName))
  {
    FREE_IMAGE_FORMAT fifmt =
      FreeImage_GetFIFFromFilename(this->dataPtr->fullName.c_str());

    if (this->dataPtr->bitmap)
      FreeImage_Unload(this->dataPtr->bitmap);
    this->dataPtr->bitmap = NULL;

    if (fifmt == FIF_PNG)
    {
      this->dataPtr->bitmap = FreeImage_Load(fifmt,
          this->dataPtr->fullName.c_str(), PNG_DEFAULT);
    }
    else if (fifmt == FIF_JPEG)
    {
      this->dataPtr->bitmap =
        FreeImage_Load(fifmt, this->dataPtr->fullName.c_str(), JPEG_DEFAULT);
    }
    else if (fifmt == FIF_BMP)
    {
      this->dataPtr->bitmap = FreeImage_Load(fifmt,
          this->dataPtr->fullName.c_str(), BMP_DEFAULT);
    }
    else
    {
      ignerr << "Unknown image format[" << this->dataPtr->fullName << "]\n";
      return -1;
    }

    return 0;
  }

  ignerr << "Unable to open image file[" << this->dataPtr->fullName
        << "], check your IGNITION_RESOURCE_PATH settings.\n";
  return -1;
}

//////////////////////////////////////////////////
int Image::Load(const std::string &_filename,
                std::optional<PixelFormatType> _outputFormat)
{
  // No target format requested: load in the source's native format.
  if (!_outputFormat.has_value())
    return this->Load(_filename);

  if (_outputFormat.value() != RGBA_INT8)
  {
    gzerr << "Unsupported target pixel format["
          << _outputFormat.value() << "] for [" << _filename
          << "]; only RGBA_INT8 (or std::nullopt for the native format) is "
          << "supported.\n";
    return -1;
  }

  this->dataPtr->fullName = _filename;
  if (!exists(this->dataPtr->fullName))
  {
    this->dataPtr->fullName = common::findFile(_filename);
  }

  if (!exists(this->dataPtr->fullName))
  {
    gzerr << "Unable to open image file[" << this->dataPtr->fullName
          << "], check your GZ_RESOURCE_PATH settings.\n";
    return -1;
  }

  if (this->dataPtr->bitmap)
    stbi_image_free(this->dataPtr->bitmap);
  this->dataPtr->bitmap = nullptr;

  // Decode straight to 8-bit RGBA in a single pass (req_comp = 4). This is
  // faster than decoding to native channels and converting afterwards -- stb's
  // SIMD path writes 4-wide RGBA more efficiently than packed RGB -- and yields
  // a texture-ready buffer with no later channel conversion. Any source format
  // (8/16-bit or HDR) is down-converted to 8-bit RGBA by stb.
  int w;
  int h;
  int n;
  void *bitmap = stbi_load(this->dataPtr->fullName.c_str(), &w, &h, &n, 4);
  if (bitmap == nullptr)
  {
    gzerr << "Failed to load file [" << this->dataPtr->fullName
          << "]: " << stbi_failure_reason() << std::endl;
    return -1;
  }

  this->dataPtr->bitmap = bitmap;
  this->dataPtr->bits_per_channel = 8;
  this->dataPtr->width = w;
  this->dataPtr->height = h;
  this->dataPtr->channels = 4;
  return 0;
}

//////////////////////////////////////////////////
void Image::SavePNG(const std::string &_filename)
{
  FreeImage_Save(FIF_PNG, this->dataPtr->bitmap, _filename.c_str(),
      PNG_DEFAULT);
}

//////////////////////////////////////////////////
void Image::SavePNGToBuffer(std::vector<unsigned char>& buffer)
{
  FIMEMORY *hmem = FreeImage_OpenMemory();
  FreeImage_SaveToMemory(FIF_PNG, this->dataPtr->bitmap, hmem);
  unsigned char *memBuffer = nullptr;
#ifndef _WIN32
  unsigned int sizeInBytes = 0;
#else
  DWORD sizeInBytes = 0;
#endif
  FreeImage_AcquireMemory(hmem, &memBuffer, &sizeInBytes);
  buffer.resize(sizeInBytes);
  std::memcpy(buffer.data(), memBuffer, sizeInBytes);
  FreeImage_CloseMemory(hmem);
}

//////////////////////////////////////////////////
void Image::SetFromData(const unsigned char *_data,
    unsigned int _width,
    unsigned int _height,
    Image::PixelFormatType _format)
{
  if (this->dataPtr->bitmap)
    FreeImage_Unload(this->dataPtr->bitmap);
  this->dataPtr->bitmap = NULL;

  // int redmask = FI_RGBA_RED_MASK;
  int redmask = 0x0000ff;

  // int greenmask = FI_RGBA_GREEN_MASK;
  int greenmask = 0x00ff00;

  // int bluemask = FI_RGBA_BLUE_MASK;
  int bluemask = 0xff0000;

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
    redmask = 0xff0000;
    greenmask = 0x00ff00;
    bluemask = 0x0000ff;
    scanlineBytes = _width * 3;
  }
  else if (_format == RGBA_INT8)
  {
    bpp = 32;
    redmask = 0xff000000;
    greenmask = 0x00ff0000;
    bluemask = 0x0000ff00;
    scanlineBytes = _width * 4;
  }
  else if (_format == BGR_INT8)
  {
    bpp = 24;
    redmask = 0x0000ff;
    greenmask = 0x00ff00;
    bluemask = 0xff0000;
    scanlineBytes = _width * 3;
  }
  else
  {
    ignerr << "Unable to handle format[" << _format << "]\n";
    return;
  }

  this->dataPtr->bitmap = FreeImage_ConvertFromRawBits(const_cast<BYTE*>(_data),
      _width, _height, scanlineBytes, bpp, redmask, greenmask, bluemask, true);

  if (this->dataPtr->ShouldSwapRedBlue())
  {
    FIBITMAP *toDelete = this->dataPtr->bitmap;
    this->dataPtr->bitmap = this->dataPtr->SwapRedBlue(this->Width(),
                                                       this->Height());
    FreeImage_Unload(toDelete);
  }
}

//////////////////////////////////////////////////
void Image::SetFromCompressedData(const unsigned char *_data,
                                  unsigned int _size,
                                  Image::PixelFormatType _inputFormat,
                                  std::optional<PixelFormatType> _outputFormat)
{
  // No target format requested: decode to the source's native format. The
  // 3-argument overload does not modify _data (stb takes it as const), so the
  // const_cast is safe.
  if (!_outputFormat.has_value())
  {
    this->SetFromCompressedData(const_cast<unsigned char *>(_data), _size,
        _inputFormat);
    return;
  }

  if (_outputFormat.value() != RGBA_INT8)
  {
    gzerr << "Unsupported target pixel format[" << _outputFormat.value()
          << "]; only RGBA_INT8 (or std::nullopt for the native format) is "
          << "supported.\n";
    return;
  }

  if (this->dataPtr->bitmap)
    stbi_image_free(this->dataPtr->bitmap);
  this->dataPtr->bitmap = nullptr;

  if (_inputFormat != COMPRESSED_PNG && _inputFormat != COMPRESSED_JPEG)
  {
    gzerr << "Unable to handle format[" << _inputFormat << "]\n";
    return;
  }

  // Decode straight to 8-bit RGBA in a single pass (req_comp = 4). \sa Load.
  int w;
  int h;
  int n;
  void *bitmap = stbi_load_from_memory(_data, _size, &w, &h, &n, 4);
  if (bitmap == nullptr)
  {
    gzerr << "Failed to decode compressed image data: "
          << stbi_failure_reason() << std::endl;
    return;
  }

  this->dataPtr->bitmap = bitmap;
  this->dataPtr->bits_per_channel = 8;
  this->dataPtr->width = w;
  this->dataPtr->height = h;
  this->dataPtr->channels = 4;
}

//////////////////////////////////////////////////
int Image::Pitch() const
{
  return FreeImage_GetLine(this->dataPtr->bitmap);
}

//////////////////////////////////////////////////
<<<<<<< HEAD
void Image::RGBData(unsigned char **_data, unsigned int &_count)
{
  FIBITMAP *tmp = this->dataPtr->bitmap;
  FIBITMAP *tmp2 = nullptr;
  if (this->dataPtr->ShouldSwapRedBlue())
  {
    tmp = this->dataPtr->SwapRedBlue(this->Width(), this->Height());
    tmp2 = tmp;
  }
  tmp = FreeImage_ConvertTo24Bits(tmp);
  this->dataPtr->DataImpl(_data, _count, tmp);
  FreeImage_Unload(tmp);
  if (tmp2)
    FreeImage_Unload(tmp2);
}

//////////////////////////////////////////////////
void Image::Data(unsigned char **_data, unsigned int &_count)
{
  if (this->dataPtr->ShouldSwapRedBlue())
  {
    FIBITMAP *tmp = this->dataPtr->SwapRedBlue(this->Width(), this->Height());
    this->dataPtr->DataImpl(_data, _count, tmp);
    FreeImage_Unload(tmp);
=======
namespace {

/// \brief Compute the luminance of an RGB triple, matching stbi__compute_y so
/// that channel reductions stay bit-identical to stb_image.
/// \tparam T Sample type (unsigned char for 8-bit, uint16_t for 16-bit).
/// \param[in] _r Red component.
/// \param[in] _g Green component.
/// \param[in] _b Blue component.
/// \return The computed luminance as type T.
template <typename T>
inline T ComputeLuminance(int _r, int _g, int _b)
{
  return static_cast<T>(((_r * 77) + (_g * 150) + (29 * _b)) >> 8);
}

/// \brief Convert pixel data from _inCh to _outCh channels in a single pass.
/// _src is never modified or freed. This mirrors stbi__convert_format /
/// stbi__convert_format16 byte-for-byte, but avoids the clone-input +
/// copy-output overhead those functions impose (they free their input, so the
/// caller previously had to duplicate the bitmap and then copy the result into
/// the returned vector).
/// \tparam T Sample type (unsigned char for 8-bit, uint16_t for 16-bit).
/// \param[in] _src Source pixels (_npix * _inCh samples).
/// \param[in] _inCh Number of channels in the source.
/// \param[out] _dst Destination buffer (_npix * _outCh samples).
/// \param[in] _outCh Number of channels to write.
/// \param[in] _npix Number of pixels to convert.
/// \param[in] _aMax Opaque alpha to insert (255 for 8-bit, 0xffff for 16-bit).
/// \return False if the channel combination is unsupported.
template <typename T>
bool ConvertChannels(const T *_src, int _inCh, T *_dst, int _outCh,
    std::size_t _npix, T _aMax)
{
  // Pack the (input, output) channel counts into one integer, _inCh * 8 +
  // _outCh, so each supported combination maps to a unique case and can be
  // dispatched by a single switch (mirrors stb_image's STBI__COMBO macro).
  switch (_inCh * 8 + _outCh)
  {
    case 1 * 8 + 2:
      for (std::size_t p = 0; p < _npix; ++p, _src += 1, _dst += 2)
      {
        _dst[0] = _src[0];
        _dst[1] = _aMax;
      }
      break;
    case 1 * 8 + 3:
      for (std::size_t p = 0; p < _npix; ++p, _src += 1, _dst += 3)
      {
        _dst[0] = _dst[1] = _dst[2] = _src[0];
      }
      break;
    case 1 * 8 + 4:
      for (std::size_t p = 0; p < _npix; ++p, _src += 1, _dst += 4)
      {
        _dst[0] = _dst[1] = _dst[2] = _src[0];
        _dst[3] = _aMax;
      }
      break;
    case 2 * 8 + 1:
      for (std::size_t p = 0; p < _npix; ++p, _src += 2, _dst += 1)
      {
        _dst[0] = _src[0];
      }
      break;
    case 2 * 8 + 3:
      for (std::size_t p = 0; p < _npix; ++p, _src += 2, _dst += 3)
      {
        _dst[0] = _dst[1] = _dst[2] = _src[0];
      }
      break;
    case 2 * 8 + 4:
      for (std::size_t p = 0; p < _npix; ++p, _src += 2, _dst += 4)
      {
        _dst[0] = _dst[1] = _dst[2] = _src[0];
        _dst[3] = _src[1];
      }
      break;
    case 3 * 8 + 4:
      for (std::size_t p = 0; p < _npix; ++p, _src += 3, _dst += 4)
      {
        _dst[0] = _src[0];
        _dst[1] = _src[1];
        _dst[2] = _src[2];
        _dst[3] = _aMax;
      }
      break;
    case 3 * 8 + 1:
      for (std::size_t p = 0; p < _npix; ++p, _src += 3, _dst += 1)
      {
        _dst[0] = ComputeLuminance<T>(_src[0], _src[1], _src[2]);
      }
      break;
    case 3 * 8 + 2:
      for (std::size_t p = 0; p < _npix; ++p, _src += 3, _dst += 2)
      {
        _dst[0] = ComputeLuminance<T>(_src[0], _src[1], _src[2]);
        _dst[1] = _aMax;
      }
      break;
    case 4 * 8 + 1:
      for (std::size_t p = 0; p < _npix; ++p, _src += 4, _dst += 1)
      {
        _dst[0] = ComputeLuminance<T>(_src[0], _src[1], _src[2]);
      }
      break;
    case 4 * 8 + 2:
      for (std::size_t p = 0; p < _npix; ++p, _src += 4, _dst += 2)
      {
        _dst[0] = ComputeLuminance<T>(_src[0], _src[1], _src[2]);
        _dst[1] = _src[3];
      }
      break;
    case 4 * 8 + 3:
      for (std::size_t p = 0; p < _npix; ++p, _src += 4, _dst += 3)
      {
        _dst[0] = _src[0];
        _dst[1] = _src[1];
        _dst[2] = _src[2];
      }
      break;
    default:
      return false;
  }
  return true;
}

}  // namespace

std::vector<unsigned char>
Image::Implementation::DataWithChannels(int out_channels) const
{
  // Nothing to convert if the image is empty or has not been loaded.
  if (this->width <= 0 || this->height <= 0 || this->bitmap == nullptr)
    return {};

  const size_t outSize = static_cast<size_t>(this->width) * this->height *
      out_channels * this->bits_per_channel / 8;
  std::vector<unsigned char> data(outSize);

  // No conversion needed: copy the bitmap straight into the output buffer.
  if (this->channels == out_channels)
  {
    memcpy(data.data(), this->bitmap, outSize);
    return data;
>>>>>>> b632fbd (Optimize texture image decoding and channel conversion (#817))
  }

  // Convert channels in a single pass directly into the output buffer.
  const std::size_t npix = static_cast<std::size_t>(this->width) * this->height;
  bool ok = false;
  switch (this->bits_per_channel)
  {
<<<<<<< HEAD
    this->dataPtr->DataImpl(_data, _count, this->dataPtr->bitmap);
=======
  case 8:
    ok = ConvertChannels(static_cast<const unsigned char *>(this->bitmap),
        this->channels, data.data(), out_channels, npix,
        static_cast<unsigned char>(255));
    break;
  case 16:
    ok = ConvertChannels(static_cast<const uint16_t *>(this->bitmap),
        this->channels, reinterpret_cast<uint16_t *>(data.data()),
        out_channels, npix, static_cast<uint16_t>(0xffff));
    break;
  default:  // 32-bit float is not supported by the channel converter
    break;
  }

  if (!ok)
  {
    gzerr << "Error converting image to " << out_channels << " channels\n";
    return std::vector<unsigned char>();
>>>>>>> b632fbd (Optimize texture image decoding and channel conversion (#817))
  }
}

//////////////////////////////////////////////////
void Image::Implementation::DataImpl(
    unsigned char **_data, unsigned int &_count, FIBITMAP *_img) const
{
  int redmask = FI_RGBA_RED_MASK;
  // int bluemask = 0x00ff0000;

  int greenmask = FI_RGBA_GREEN_MASK;
  // int greenmask = 0x0000ff00;

  int bluemask = FI_RGBA_BLUE_MASK;
  // int redmask = 0x000000ff;

  int scanWidth = FreeImage_GetLine(_img);

  if (*_data)
    delete [] *_data;

  _count = scanWidth * FreeImage_GetHeight(_img);
  *_data = new unsigned char[_count];

  FreeImage_ConvertToRawBits(reinterpret_cast<BYTE*>(*_data), _img,
      scanWidth, FreeImage_GetBPP(_img), redmask, greenmask, bluemask, true);

#ifdef FREEIMAGE_COLORORDER
  // cppcheck-suppress ConfigurationNotChecked
  if (FREEIMAGE_COLORORDER != FREEIMAGE_COLORORDER_RGB)
  {
#else
#ifdef FREEIMAGE_BIGENDIAN
  if (false)
  {
#else
  {
#endif
#endif
//  FIXME:  why shift by 2 pixels?
//  this breaks heighmaps by wrapping artificially
//    int i = 0;
//    for (unsigned int y = 0; y < this->Height(); ++y)
//    {
//      for (unsigned int x = 0; x < this->Width(); ++x)
//      {
//        std::swap((*_data)[i], (*_data)[i+2]);
//        unsigned int d = FreeImage_GetBPP(this->dataPtr->bitmap)/8;
//        i += d;
//      }
//    }
  }
}

//////////////////////////////////////////////////
unsigned int Image::Width() const
{
  if (!this->Valid())
    return 0;

  return FreeImage_GetWidth(this->dataPtr->bitmap);
}

//////////////////////////////////////////////////
unsigned int Image::Height() const
{
  if (!this->Valid())
    return 0;

  return FreeImage_GetHeight(this->dataPtr->bitmap);
}

//////////////////////////////////////////////////
unsigned int Image::BPP() const
{
  if (!this->Valid())
    return 0;

  return FreeImage_GetBPP(this->dataPtr->bitmap);
}

//////////////////////////////////////////////////
math::Color Image::Pixel(unsigned int _x, unsigned int _y) const
{
  math::Color clr;

  if (!this->Valid())
    return clr;

  FREE_IMAGE_COLOR_TYPE type = FreeImage_GetColorType(this->dataPtr->bitmap);

  if (type == FIC_RGB || type == FIC_RGBALPHA)
  {
    RGBQUAD firgb;

    if (FreeImage_GetPixelColor(this->dataPtr->bitmap, _x, _y, &firgb) == FALSE)
    {
      ignerr << "Image: Coordinates out of range["
        << _x << " " << _y << "] \n";
      return clr;
    }
    clr.Set(firgb.rgbRed, firgb.rgbGreen, firgb.rgbBlue);
  }
  else
  {
    BYTE byteValue;
    if (FreeImage_GetPixelIndex(
          this->dataPtr->bitmap, _x, _y, &byteValue) == FALSE)
    {
      ignerr << "Image: Coordinates out of range ["
        << _x << " " << _y << "] \n";
      return clr;
    }

    clr.Set(byteValue, byteValue, byteValue);
  }

  return clr;
}

//////////////////////////////////////////////////
math::Color Image::AvgColor()
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
  unsigned int x, y;
  math::Color clr;
  math::Color maxClr;

  maxClr.Set(0, 0, 0, 0);

  if (!this->Valid())
    return clr;

  FREE_IMAGE_COLOR_TYPE type = FreeImage_GetColorType(this->dataPtr->bitmap);

  if (type == FIC_RGB || type == FIC_RGBALPHA)
  {
    RGBQUAD firgb;

    for (y = 0; y < this->Height(); y++)
    {
      for (x = 0; x < this->Width(); x++)
      {
        clr.Set(0, 0, 0, 0);

        if (FALSE ==
              FreeImage_GetPixelColor(this->dataPtr->bitmap, x, y, &firgb))
        {
          ignerr << "Image: Coordinates out of range["
            << x << " " << y << "] \n";
          continue;
        }
        clr.Set(firgb.rgbRed, firgb.rgbGreen, firgb.rgbBlue);

        if (clr.R() + clr.G() + clr.B() > maxClr.R() + maxClr.G() + maxClr.B())
        {
          maxClr = clr;
        }
      }
    }
  }
  else
  {
    BYTE byteValue;
    for (y = 0; y < this->Height(); y++)
    {
      for (x = 0; x < this->Width(); x++)
      {
        clr.Set(0, 0, 0, 0);

        if (FreeImage_GetPixelIndex(
              this->dataPtr->bitmap, x, y, &byteValue) == FALSE)
        {
          ignerr << "Image: Coordinates out of range ["
            << x << " " << y << "] \n";
          continue;
        }

        clr.Set(byteValue, byteValue, byteValue);

        if (clr.R() + clr.G() + clr.B() > maxClr.R() + maxClr.G() + maxClr.B())
        {
          maxClr = clr;
        }
      }
    }
  }

  return maxClr;
}

//////////////////////////////////////////////////
void Image::Rescale(int _width, int _height)
{
  this->dataPtr->bitmap = FreeImage_Rescale(
      this->dataPtr->bitmap, _width, _height, FILTER_LANCZOS3);
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
  Image::PixelFormatType fmt = UNKNOWN_PIXEL_FORMAT;
  FREE_IMAGE_TYPE type = FreeImage_GetImageType(this->dataPtr->bitmap);

  unsigned int redMask = FreeImage_GetRedMask(this->dataPtr->bitmap);
  unsigned int bpp = this->BPP();

  if (type == FIT_BITMAP)
  {
    if (bpp == 8)
      fmt = L_INT8;
    else if (bpp == 16)
      fmt = L_INT16;
    else if (bpp == 24)
      redMask == 0xff0000 ? fmt = RGB_INT8 : fmt = BGR_INT8;
    else if (bpp == 32)
    {
      redMask == 0xff0000 || redMask == 0xff000000 ?
        fmt = RGBA_INT8 : fmt = BGRA_INT8;
    }
  }
  else if (type == FIT_RGB16)
    fmt = RGB_INT16;
  else if (type == FIT_RGBF)
    fmt = RGB_FLOAT32;
  else if (type == FIT_UINT16 || type == FIT_INT16)
    fmt = L_INT16;

  return fmt;
}

/////////////////////////////////////////////////
Image::PixelFormatType Image::ConvertPixelFormat(const std::string &_format)
{
  // Handle old format strings
  if (_format == "L8" || _format == "L_INT8")
    return L_INT8;
  else if (_format == "R8G8B8" || _format == "RGB_INT8")
    return RGB_INT8;

  // Handle BAYER_BGGR8 since it is after PIXEL_FORMAT_COUNT in the enum
  if (_format == "BAYER_BGGR8")
  {
    return BAYER_BGGR8;
  }

  for (unsigned int i = 0; i < PIXEL_FORMAT_COUNT; ++i)
    if (PixelFormatNames[i] == _format)
      return static_cast<PixelFormatType>(i);

  return UNKNOWN_PIXEL_FORMAT;
}

//////////////////////////////////////////////////
bool Image::Implementation::ShouldSwapRedBlue() const
{
  return CanSwapRedBlue() && FREEIMAGE_COLORORDER != FREEIMAGE_COLORORDER_RGB;
}

//////////////////////////////////////////////////
bool Image::Implementation::CanSwapRedBlue() const
{
  const unsigned bpp = FreeImage_GetBPP(this->bitmap);
  return bpp == 24u || bpp == 32u;
}

//////////////////////////////////////////////////
FIBITMAP* Image::Implementation::SwapRedBlue(const unsigned int &_width,
                                    const unsigned int &_height)
{
  FIBITMAP *copy = FreeImage_Copy(this->bitmap, 0, 0, _width, _height);

  const unsigned bytesperpixel = FreeImage_GetBPP(this->bitmap) / 8;
  const unsigned pitch = FreeImage_GetPitch(this->bitmap);
  const unsigned lineSize = FreeImage_GetLine(this->bitmap);

  BYTE *line = FreeImage_GetBits(copy);
  for (unsigned y = 0; y < _height; ++y, line += pitch)
  {
    for (BYTE *pixel = line; pixel < line + lineSize ; pixel += bytesperpixel)
    {
      std::swap(pixel[0], pixel[2]);
    }
  }

  return copy;
}
