/*
 * Copyright (C) 2012-2014 Open Source Robotics Foundation
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

#include <boost/filesystem.hpp>
#include <string>

#include <ignition/common/Console.hh>
#include <ignition/common/Util.hh>
#include <ignition/common/Image.hh>
#include <ignition/common/ImagePrivate.hh>

using namespace ignition;
using namespace common;

static int count = 0;

//////////////////////////////////////////////////
Image::Image(const std::string &_filename)
  : data(new ImagePrivate)
{
  if (count == 0)
    FreeImage_Initialise();

  count++;

  this->data->bitmap = NULL;
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

  if (this->data->bitmap)
    FreeImage_Unload(this->data->bitmap);
  this->data->bitmap = NULL;

  if (count == 0)
    FreeImage_DeInitialise();

  delete this->data;
  this->data = NULL;
}


//////////////////////////////////////////////////
int Image::Load(const std::string &_filename)
{
  this->data->fullName = _filename;
  if (!boost::filesystem::exists(boost::filesystem::path(this->data->fullName)))
    this->data->fullName = common::findFile(_filename);

  if (boost::filesystem::exists(boost::filesystem::path(this->data->fullName)))
  {
    FREE_IMAGE_FORMAT fifmt =
      FreeImage_GetFIFFromFilename(this->data->fullName.c_str());

    if (this->data->bitmap)
      FreeImage_Unload(this->data->bitmap);
    this->data->bitmap = NULL;

    if (fifmt == FIF_PNG)
    {
      this->data->bitmap = FreeImage_Load(fifmt,
          this->data->fullName.c_str(), PNG_DEFAULT);
    }
    else if (fifmt == FIF_JPEG)
    {
      this->data->bitmap =
        FreeImage_Load(fifmt, this->data->fullName.c_str(), JPEG_DEFAULT);
    }
    else if (fifmt == FIF_BMP)
    {
      this->data->bitmap = FreeImage_Load(fifmt,
          this->data->fullName.c_str(), BMP_DEFAULT);
    }
    else
    {
      ignerr << "Unknown image format[" << this->data->fullName << "]\n";
      return -1;
    }

    return 0;
  }

  ignerr << "Unable to open image file[" << this->data->fullName
        << "], check your IGNITION_RESOURCE_PATH settings.\n";
  return -1;
}

//////////////////////////////////////////////////
void Image::SavePNG(const std::string &_filename)
{
  FreeImage_Save(FIF_PNG, this->data->bitmap, _filename.c_str(), 0);
}

//////////////////////////////////////////////////
void Image::SetFromData(const unsigned char *_data, unsigned int _width,
    unsigned int _height, PixelFormat _format)
{
  if (this->data->bitmap)
    FreeImage_Unload(this->data->bitmap);
  this->data->bitmap = NULL;

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

  this->data->bitmap = FreeImage_ConvertFromRawBits(const_cast<BYTE*>(_data),
      _width, _height, scanlineBytes, bpp, redmask, greenmask, bluemask, true);
}

//////////////////////////////////////////////////
int Image::GetPitch() const
{
  return FreeImage_GetLine(this->data->bitmap);
}

//////////////////////////////////////////////////
void Image::GetRGBData(unsigned char **_data, unsigned int &_count) const
{
  FIBITMAP *tmp = FreeImage_ConvertTo24Bits(this->data->bitmap);
  this->GetDataImpl(_data, _count, tmp);
  FreeImage_Unload(tmp);
}

//////////////////////////////////////////////////
void Image::GetData(unsigned char **_data, unsigned int &_count) const
{
  this->GetDataImpl(_data, _count, this->data->bitmap);
}

//////////////////////////////////////////////////
void Image::GetDataImpl(unsigned char **_data, unsigned int &_count,
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
/*  FIXME:  why shift by 2 pixels?  this breaks heighmaps by wrapping artificially
    int i = 0;
    for (unsigned int y = 0; y < this->GetHeight(); ++y)
    {
      for (unsigned int x = 0; x < this->GetWidth(); ++x)
      {
        std::swap((*_data)[i], (*_data)[i+2]);
        unsigned int d = FreeImage_GetBPP(this->data->bitmap)/8;
        i += d;
      }
    }
*/
  }
}

//////////////////////////////////////////////////
unsigned int Image::GetWidth() const
{
  if (!this->Valid())
    return 0;

  return FreeImage_GetWidth(this->data->bitmap);
}

//////////////////////////////////////////////////
unsigned int Image::GetHeight() const
{
  if (!this->Valid())
    return 0;

  return FreeImage_GetHeight(this->data->bitmap);
}

//////////////////////////////////////////////////
unsigned int Image::GetBPP() const
{
  if (!this->Valid())
    return 0;

  return FreeImage_GetBPP(this->data->bitmap);
}

//////////////////////////////////////////////////
Color Image::GetPixel(unsigned int _x, unsigned int _y) const
{
  Color clr;

  if (!this->Valid())
    return clr;

  FREE_IMAGE_COLOR_TYPE type = FreeImage_GetColorType(this->data->bitmap);

  if (type == FIC_RGB || type == FIC_RGBALPHA)
  {
    RGBQUAD firgb;

    if (FreeImage_GetPixelColor(this->data->bitmap, _x, _y, &firgb) == FALSE)
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
          this->data->bitmap, _x, _y, &byteValue) == FALSE)
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
Color Image::GetAvgColor()
{
  unsigned int x, y;
  double rsum, gsum, bsum;
  common::Color pixel;

  rsum = gsum = bsum = 0.0;
  for (y = 0; y < this->GetHeight(); ++y)
  {
    for (x = 0; x < this->GetWidth(); ++x)
    {
      pixel = this->GetPixel(x, y);
      rsum += pixel.R();
      gsum += pixel.G();
      bsum += pixel.B();
    }
  }

  rsum /= (this->GetWidth() * this->GetHeight());
  gsum /= (this->GetWidth() * this->GetHeight());
  bsum /= (this->GetWidth() * this->GetHeight());

  return Color(rsum, gsum, bsum);
}

//////////////////////////////////////////////////
Color Image::GetMaxColor() const
{
  unsigned int x, y;
  Color clr;
  Color maxClr;

  maxClr.Set(0, 0, 0, 0);

  for (y = 0; y < this->GetHeight(); y++)
  {
    for (x = 0; x < this->GetWidth(); x++)
    {
      clr = this->GetPixel(x, y);

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
  this->data->bitmap = FreeImage_Rescale(this->data->bitmap, _width, _height,
      FILTER_LANCZOS3);
}

//////////////////////////////////////////////////
bool Image::Valid() const
{
  return this->data->bitmap != NULL;
}

//////////////////////////////////////////////////
std::string Image::GetFilename() const
{
  return this->data->fullName;
}

//////////////////////////////////////////////////
Image::PixelFormat Image::GetPixelFormat() const
{
  Image::PixelFormat fmt = UNKNOWN_PIXEL_FORMAT;
  FREE_IMAGE_TYPE type = FreeImage_GetImageType(this->data->bitmap);

  unsigned int redMask = FreeImage_GetRedMask(this->data->bitmap);
  unsigned int bpp = this->GetBPP();

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
Image::PixelFormat Image::ConvertPixelFormat(const std::string &_format)
{
  // Handle old format strings
  if (_format == "L8" || _format == "L_INT8")
    return L_INT8;
  else if (_format == "R8G8B8" || _format == "RGB_INT8")
    return RGB_INT8;

  for (unsigned int i = 0; i < PIXEL_FORMAT_COUNT; ++i)
    if (PixelFormatNames[i] == _format)
      return static_cast<PixelFormat>(i);

  return UNKNOWN_PIXEL_FORMAT;
}
