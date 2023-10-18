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
      /// \brief bitmap data
      public: FIBITMAP *bitmap;

      /// \brief path name of the image file
      public: std::string fullName;

      /// \brief Implementation of GetData
      /// \deprecated remove once the Data functions using raw pointers
      /// are removed, in favor of returning vectors of bytes
      public: void DataImpl(unsigned char **_data, unsigned int &_count,
          FIBITMAP *_img) const;

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

    return 0;
  }

  gzerr << "Unable to open image file[" << this->dataPtr->fullName
        << "], check your GZ_RESOURCE_PATH settings.\n";
  return -1;
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
  }
  else
  {
    gzerr << "Unable to handle format[" << _format << "]\n";
    return;
  }
}

//////////////////////////////////////////////////
int Image::Pitch() const
{
  return FreeImage_GetLine(this->dataPtr->bitmap);
}

//////////////////////////////////////////////////
void Image::RGBData(unsigned char **_data, unsigned int &_count) const
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
void Image::RGBAData(unsigned char **_data, unsigned int &_count) const
{
  FIBITMAP *tmp = this->dataPtr->bitmap;
  FIBITMAP *tmp2 = nullptr;
  if (this->dataPtr->ShouldSwapRedBlue())
  {
    tmp = this->dataPtr->SwapRedBlue(this->Width(), this->Height());
    tmp2 = tmp;
  }
  tmp = FreeImage_ConvertTo32Bits(tmp);
  this->dataPtr->DataImpl(_data, _count, tmp);
  FreeImage_Unload(tmp);
  if (tmp2)
    FreeImage_Unload(tmp2);
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
void Image::Data(unsigned char **_data, unsigned int &_count) const
{
  if (this->dataPtr->ShouldSwapRedBlue())
  {
    FIBITMAP *tmp = this->dataPtr->SwapRedBlue(this->Width(), this->Height());
    this->dataPtr->DataImpl(_data, _count, tmp);
    FreeImage_Unload(tmp);
  }
  else
  {
    this->dataPtr->DataImpl(_data, _count, this->dataPtr->bitmap);
  }
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
      gzerr << "Image: Coordinates out of range["
        << _x << " " << _y << "] \n";
      return clr;
    }
    clr.Set(firgb.rgbRed, firgb.rgbGreen, firgb.rgbBlue);
  }
  else
  {
    if (this->dataPtr->PixelIndex(
           this->dataPtr->bitmap, _x, _y, clr) == FALSE)
    {
      gzerr << "Image: Coordinates out of range ["
        << _x << " " << _y << "] \n";
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
          gzerr << "Image: Coordinates out of range["
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
    for (y = 0; y < this->Height(); y++)
    {
      for (x = 0; x < this->Width(); x++)
      {
        clr.Set(0, 0, 0, 0);

        if (this->dataPtr->PixelIndex(
               this->dataPtr->bitmap, x, y, clr) == FALSE)
        {
          gzerr << "Image: Coordinates out of range ["
            << x << " " << y << "] \n";
          continue;
        }

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
BOOL Image::Implementation::PixelIndex(
    FIBITMAP *_dib, unsigned _x, unsigned _y, math::Color &_color) const
{
  if (!_dib)
    return FALSE;

  FREE_IMAGE_TYPE imageType = FreeImage_GetImageType(_dib);
  // 8 bit images
  if (imageType == FIT_BITMAP)
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
  else if (imageType == FIT_UINT16)
  {
    if ((_x < FreeImage_GetWidth(_dib)) && (_y < FreeImage_GetHeight(_dib)))
    {
      WORD *bits = reinterpret_cast<WORD *>(FreeImage_GetScanLine(_dib, _y));
      uint16_t word = static_cast<uint16_t>(bits[_x]);
      // convert to float value between 0-1
      float value = word / static_cast<float>(math::MAX_UI16);
      _color.Set(value, value, value);
    }
    else
    {
      return FALSE;
    }
  }
  return TRUE;
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
