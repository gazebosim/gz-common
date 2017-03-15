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

#include <string>

#include <ignition/common/Console.hh>
#include <ignition/common/Util.hh>
#include <ignition/common/Image.hh>

using namespace ignition;
using namespace common;

namespace ignition
{
  namespace common
  {
    /// \brief Private data class
    class ImagePrivate
    {
      /// \brief bitmap data
      public: FIBITMAP *bitmap;

      /// \brief path name of the image file
      public: std::string fullName;

      /// \brief Implementation of GetData
      public: void DataImpl(unsigned char **_data, unsigned int &_count,
          FIBITMAP *_img) const;
    };
  }
}

static int count = 0;

//////////////////////////////////////////////////
Image::Image(const std::string &_filename)
  : dataPtr(new ImagePrivate)
{
  if (count == 0)
    FreeImage_Initialise();

  count++;

  this->dataPtr->bitmap = NULL;
  if (!_filename.empty())
  {
    std::string filename = ignition::common::findFile(_filename);
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
void Image::SavePNG(const std::string &_filename)
{
  FreeImage_Save(FIF_PNG, this->dataPtr->bitmap, _filename.c_str(), 0);
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
}

//////////////////////////////////////////////////
int Image::Pitch() const
{
  return FreeImage_GetLine(this->dataPtr->bitmap);
}

//////////////////////////////////////////////////
void Image::RGBData(unsigned char **_data, unsigned int &_count) const
{
  FIBITMAP *tmp = FreeImage_ConvertTo24Bits(this->dataPtr->bitmap);
  this->dataPtr->DataImpl(_data, _count, tmp);
  FreeImage_Unload(tmp);
}

//////////////////////////////////////////////////
void Image::Data(unsigned char **_data, unsigned int &_count) const
{
  this->dataPtr->DataImpl(_data, _count, this->dataPtr->bitmap);
}

//////////////////////////////////////////////////
void ImagePrivate::DataImpl(unsigned char **_data, unsigned int &_count,
                        FIBITMAP *_img) const
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

#ifdef FREEIMAGE_COLORORDER
    if (FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_RGB)
      clr.Set(firgb.rgbRed, firgb.rgbGreen, firgb.rgbBlue);
    else
      clr.Set(firgb.rgbBlue, firgb.rgbGreen, firgb.rgbRed);
#else
#ifdef FREEIMAGE_BIGENDIAN
    clr.Set(firgb.rgbRed, firgb.rgbGreen, firgb.rgbBlue);
#else
    clr.Set(firgb.rgbBlue, firgb.rgbGreen, firgb.rgbRed);
#endif
#endif
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

  for (y = 0; y < this->Height(); y++)
  {
    for (x = 0; x < this->Width(); x++)
    {
      clr = this->Pixel(x, y);

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

  for (unsigned int i = 0; i < PIXEL_FORMAT_COUNT; ++i)
    if (PixelFormatNames[i] == _format)
      return static_cast<PixelFormatType>(i);

  return UNKNOWN_PIXEL_FORMAT;
}
