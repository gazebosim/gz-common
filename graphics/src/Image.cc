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

#include <cstdint>
#include <cstring>
#include <optional>
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
      public: FIBITMAP *bitmap{nullptr};

      /// \brief path name of the image file
      public: std::string fullName;

<<<<<<< HEAD
      /// \brief Color type for this image
      public: FREE_IMAGE_COLOR_TYPE colorType{FIC_RGB};

      /// \brief Image type, i.e. pixel format
      public: FREE_IMAGE_TYPE imageType{FIT_UNKNOWN};
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
>>>>>>> b632fbd (Optimize texture image decoding and channel conversion (#817))

      /// \brief Implementation of Data, returns vector of bytes
      public: std::vector<unsigned char> DataImpl(FIBITMAP *_img) const;

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
                                    const unsigned int &_height) const;

      /// \brief Get pixel value at specified index.
      /// \param[in] _dib Pointer to Freeimage bitmap
      /// \param[in] _x Pixel index in horizontal direction
      /// \param[in] _y Pixel index in vertical direction
      /// \param[out] _color Pixel value at specified index
      /// \return TRUE value if the pixel index was found and the color
      /// value set, FALSE otherwise.
      public: BOOL PixelIndex(FIBITMAP *_dib, unsigned _x, unsigned _y,
            math::Color &_color) const;
    };
  }
}

static int count = 0;

//////////////////////////////////////////////////
Image::Image(const std::string &_filename)
: dataPtr(gz::utils::MakeImpl<Implementation>())
{
  if (count == 0)
    FreeImage_Initialise();

  count++;

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
    this->dataPtr->fullName = common::findFile(_filename);
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
      gzerr << "Unknown image format[" << this->dataPtr->fullName << "]\n";
      return -1;
    }

    this->dataPtr->colorType = FreeImage_GetColorType(this->dataPtr->bitmap);
    this->dataPtr->imageType = FreeImage_GetImageType(this->dataPtr->bitmap);

    return 0;
  }

  gzerr << "Unable to open image file[" << this->dataPtr->fullName
        << "], check your GZ_RESOURCE_PATH settings.\n";
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
  else if ((_format == BAYER_RGGB8) ||
           (_format == BAYER_BGGR8) ||
           (_format == BAYER_GBRG8) ||
           (_format == BAYER_GRBG8))
  {
    bpp = 8;
    scanlineBytes = _width;
  }
  else
  {
    gzerr << "Unable to handle format[" << _format << "]\n";
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
  this->dataPtr->colorType = FreeImage_GetColorType(this->dataPtr->bitmap);
  this->dataPtr->imageType = FreeImage_GetImageType(this->dataPtr->bitmap);
}

//////////////////////////////////////////////////
void Image::SetFromCompressedData(unsigned char *_data,
                                  unsigned int _size,
                                  Image::PixelFormatType _format)
{
  if (this->dataPtr->bitmap)
    FreeImage_Unload(this->dataPtr->bitmap);
  this->dataPtr->bitmap = nullptr;

  FREE_IMAGE_FORMAT format = FIF_UNKNOWN;
  switch (_format)
  {
    case COMPRESSED_PNG:
      format = FIF_PNG;
      break;
    case COMPRESSED_JPEG:
      format = FIF_JPEG;
      break;
    default:
      break;
  }
  if (format != FIF_UNKNOWN)
  {
    FIMEMORY *fiMem = FreeImage_OpenMemory(_data, _size);
    this->dataPtr->bitmap = FreeImage_LoadFromMemory(format, fiMem);
    FreeImage_CloseMemory(fiMem);
    this->dataPtr->colorType = FreeImage_GetColorType(this->dataPtr->bitmap);
    this->dataPtr->imageType = FreeImage_GetImageType(this->dataPtr->bitmap);
  }
  else
  {
    gzerr << "Unable to handle format[" << _format << "]\n";
    return;
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
<<<<<<< HEAD
  return FreeImage_GetLine(this->dataPtr->bitmap);
=======
  return this->dataPtr->width * this->dataPtr->channels *
         this->dataPtr->bits_per_channel / 8;
}

//////////////////////////////////////////////////
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
  }

  // Convert channels in a single pass directly into the output buffer.
  const std::size_t npix = static_cast<std::size_t>(this->width) * this->height;
  bool ok = false;
  switch (this->bits_per_channel)
  {
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
  }
  return data;
>>>>>>> b632fbd (Optimize texture image decoding and channel conversion (#817))
}

//////////////////////////////////////////////////
std::vector<unsigned char> Image::RGBData() const
{
  std::vector<unsigned char> data;

  FIBITMAP *tmp = this->dataPtr->bitmap;
  FIBITMAP *tmp2 = nullptr;
  if (this->dataPtr->ShouldSwapRedBlue())
  {
    tmp = this->dataPtr->SwapRedBlue(this->Width(), this->Height());
    tmp2 = tmp;
  }
  tmp = FreeImage_ConvertTo24Bits(tmp);
  data = this->dataPtr->DataImpl(tmp);
  FreeImage_Unload(tmp);
  if (tmp2)
    FreeImage_Unload(tmp2);

  return data;
}

//////////////////////////////////////////////////
std::vector<unsigned char> Image::RGBAData() const
{
  std::vector<unsigned char> data;

  FIBITMAP *tmp = this->dataPtr->bitmap;
  FIBITMAP *tmp2 = nullptr;
  if (this->dataPtr->ShouldSwapRedBlue())
  {
    tmp = this->dataPtr->SwapRedBlue(this->Width(), this->Height());
    tmp2 = tmp;
  }
  tmp = FreeImage_ConvertTo32Bits(tmp);
  data = this->dataPtr->DataImpl(tmp);
  FreeImage_Unload(tmp);
  if (tmp2)
    FreeImage_Unload(tmp2);

  return data;
}

//////////////////////////////////////////////////
std::vector<unsigned char> Image::Data() const
{
  std::vector<unsigned char> data;
  if (this->dataPtr->ShouldSwapRedBlue())
  {
    FIBITMAP *tmp = this->dataPtr->SwapRedBlue(this->Width(), this->Height());
    data = this->dataPtr->DataImpl(tmp);
    FreeImage_Unload(tmp);
  }
  else
  {
    data = this->dataPtr->DataImpl(this->dataPtr->bitmap);
  }
  return data;
}

//////////////////////////////////////////////////
std::vector<unsigned char> Image::Implementation::DataImpl(FIBITMAP *_img) const
{
  int redmask = FI_RGBA_RED_MASK;
  // int bluemask = 0x00ff0000;

  int greenmask = FI_RGBA_GREEN_MASK;
  // int greenmask = 0x0000ff00;

  int bluemask = FI_RGBA_BLUE_MASK;
  // int redmask = 0x000000ff;

  int scanWidth = FreeImage_GetLine(_img);

  std::vector<unsigned char> data(scanWidth * FreeImage_GetHeight(_img));

  FreeImage_ConvertToRawBits(reinterpret_cast<BYTE*>(&data[0]), _img,
      scanWidth, FreeImage_GetBPP(_img), redmask, greenmask, bluemask, true);

  return data;
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

  if (_x >= this->Width() || _y >= this->Height())
  {
    gzerr << "Image: Coordinates out of range["
      << _x << ", " << _y << "] \n";
    return clr;
  }

  if ((this->dataPtr->colorType == FIC_RGB ||
       this->dataPtr->colorType == FIC_RGBALPHA) &&
      (this->dataPtr->imageType == FIT_BITMAP))
  {
    RGBQUAD firgb;
    if (FreeImage_GetPixelColor(this->dataPtr->bitmap, _x, _y, &firgb) == FALSE)
    {
      gzerr << "Failed to get pixel value at ["
            << _x << ", " << _y << "] \n";
      return clr;
    }
    clr.Set(firgb.rgbRed / 255.0f, firgb.rgbGreen / 255.0f,
            firgb.rgbBlue / 255.0f);
  }
  else
  {
    if (this->dataPtr->PixelIndex(
           this->dataPtr->bitmap, _x, _y, clr) == FALSE)
    {
      gzerr << "Failed to get pixel value at ["
            << _x << ", " << _y << "] \n";
      return clr;
    }
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
BOOL Image::Implementation::PixelIndex(
    FIBITMAP *_dib, unsigned _x, unsigned _y, math::Color &_color) const
{
  if (!_dib)
    return FALSE;

  if (_x >= FreeImage_GetWidth(_dib) || _y >= FreeImage_GetHeight(_dib))
    return FALSE;

  // 8 bit images
  if (this->imageType == FIT_BITMAP)
  {
    BYTE byteValue;
    // FreeImage_GetPixelIndex should also work with 1 and 4 bit images
    if (FreeImage_GetPixelIndex(
        _dib, _x, _y, &byteValue) == FALSE)
    {
      return FALSE;
    }

    unsigned int bpp = FreeImage_GetBPP(_dib);
    // convert to float value between 0-1
    float value = byteValue / static_cast<float>(((1 << (bpp)) - 1));
    _color.Set(value, value, value);
  }
  // 16 bit images
  else if (this->imageType == FIT_UINT16)
  {
    WORD *bits = reinterpret_cast<WORD *>(FreeImage_GetScanLine(_dib, _y));
    uint16_t word = static_cast<uint16_t>(bits[_x]);
    // convert to float value between 0-1
    float value = word / static_cast<float>(math::MAX_UI16);
    _color.Set(value, value, value);
  }
  else if (this->imageType == FIT_INT16)
  {
    WORD *bits = reinterpret_cast<WORD *>(FreeImage_GetScanLine(_dib, _y));
    int16_t word = static_cast<int16_t>(bits[_x]);
    // convert to float value between 0-1
    float value = word / static_cast<float>(math::MAX_I16);
    _color.Set(value, value, value);
  }
  else if (this->imageType == FIT_RGB16)
  {
    const unsigned int channels = 3u;
    WORD *bits = reinterpret_cast<WORD *>(FreeImage_GetScanLine(_dib, _y));
    uint16_t r = static_cast<uint16_t>(bits[_x * channels]);
    uint16_t g = static_cast<uint16_t>(bits[_x * channels + 1]);
    uint16_t b = static_cast<uint16_t>(bits[_x * channels + 2]);
    // convert to float value between 0-1
    float valueR = r / static_cast<float>(math::MAX_UI16);
    float valueG = g / static_cast<float>(math::MAX_UI16);
    float valueB = b / static_cast<float>(math::MAX_UI16);
    _color.Set(valueR, valueG, valueB);
  }
  else if (this->imageType == FIT_RGBA16)
  {
    const unsigned int channels = 4u;
    WORD *bits = reinterpret_cast<WORD *>(FreeImage_GetScanLine(_dib, _y));
    uint16_t r = static_cast<uint16_t>(bits[_x * channels]);
    uint16_t g = static_cast<uint16_t>(bits[_x * channels + 1]);
    uint16_t b = static_cast<uint16_t>(bits[_x * channels + 2]);
    uint16_t a = static_cast<uint16_t>(bits[_x * channels + 3]);
    // convert to float value between 0-1
    float valueR = r / static_cast<float>(math::MAX_UI16);
    float valueG = g / static_cast<float>(math::MAX_UI16);
    float valueB = b / static_cast<float>(math::MAX_UI16);
    float valueA = a / static_cast<float>(math::MAX_UI16);
    _color.Set(valueR, valueG, valueB, valueA);
  }

  return TRUE;
}

//////////////////////////////////////////////////
void Image::Rescale(int _width, int _height)
{
  auto *scaled = FreeImage_Rescale(
      this->dataPtr->bitmap, _width, _height, FILTER_LANCZOS3);

  if (!scaled)
  {
    gzerr << "Failed to rescale image\n";
    return;
  }

  FreeImage_Unload(this->dataPtr->bitmap);
  this->dataPtr->bitmap = scaled;
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
                                    const unsigned int &_height) const
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

//////////////////////////////////////////////////
std::vector<unsigned char> Image::ChannelData(Channel _channel) const
{
  if (!this->Valid())
    return {};

  // If the image is already 8-bit grayscale, return the whole data buffer.
  if (this->PixelFormat() == L_INT8)
  {
    if (_channel == Channel::ALPHA)
    {
      return std::vector<unsigned char>(this->Width() * this->Height(), 255);
    }
    return this->Data();
  }

  FIBITMAP* channelData = nullptr;
  switch (_channel)
  {
    case Channel::RED:
      channelData = FreeImage_GetChannel(this->dataPtr->bitmap, FICC_RED);
      break;
    case Channel::GREEN:
      channelData = FreeImage_GetChannel(this->dataPtr->bitmap, FICC_GREEN);
      break;
    case Channel::BLUE:
      channelData = FreeImage_GetChannel(this->dataPtr->bitmap, FICC_BLUE);
      break;
    case Channel::ALPHA:
      channelData = FreeImage_GetChannel(this->dataPtr->bitmap, FICC_ALPHA);
      break;
    default:
      break;
  }

  if (!channelData)
  {
    gzerr << "Failed to extract channel data for input channel: "
          << static_cast<int>(_channel) << std::endl;
    return {};
  }

  FIBITMAP *tmp = FreeImage_ConvertTo8Bits(channelData);
  std::vector<unsigned char> data = this->dataPtr->DataImpl(tmp);
  FreeImage_Unload(tmp);
  FreeImage_Unload(channelData);
  return data;
}
