/*
 * Copyright (C) 2023 Open Source Robotics Foundation
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

#include "ImageLoaderFreeImage.hh"

#include <FreeImage.h>

#include <gz/common/Console.hh>

namespace
{
bool CanSwapRedBlue(FIBITMAP *_bitmap)
{
  const unsigned bpp = FreeImage_GetBPP(_bitmap);
  return bpp == 24u || bpp == 32u;
}

bool ShouldSwapRedBlue(FIBITMAP *_bitmap)
{
  return CanSwapRedBlue(_bitmap) &&
      FREEIMAGE_COLORORDER != FREEIMAGE_COLORORDER_RGB;
}

//////////////////////////////////////////////////
FIBITMAP* SwapRedBlue(FIBITMAP *_bitmap)
{
  auto  width = FreeImage_GetWidth(_bitmap);
  auto height = FreeImage_GetHeight(_bitmap);

  FIBITMAP *copy = FreeImage_Copy(_bitmap, 0, 0, width, height);

  const unsigned bytesperpixel = FreeImage_GetBPP(_bitmap) / 8;
  const unsigned pitch = FreeImage_GetPitch(_bitmap);
  const unsigned lineSize = FreeImage_GetLine(_bitmap);

  BYTE *line = FreeImage_GetBits(copy);
  for (unsigned y = 0; y < height; ++y, line += pitch)
  {
    for (BYTE *pixel = line; pixel < line + lineSize ; pixel += bytesperpixel)
    {
      std::swap(pixel[0], pixel[2]);
    }
  }

  return copy;
}

std::vector<unsigned char> DataImpl(FIBITMAP *_img)
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
std::vector<unsigned char> RGBAData(FIBITMAP *_img)
{
  std::vector<unsigned char> data;

  FIBITMAP *tmp = _img;
  FIBITMAP *tmp2 = nullptr;
  if (ShouldSwapRedBlue(_img))
  {
    tmp = SwapRedBlue(_img);
    tmp2 = tmp;
  }
  tmp = FreeImage_ConvertTo32Bits(tmp);
  data = DataImpl(tmp);
  FreeImage_Unload(tmp);
  if (tmp2)
    FreeImage_Unload(tmp2);

  return data;
}

}  // namespace

namespace gz::common
{

ImageLoaderFreeImage::ImageLoaderFreeImage()
{
  FreeImage_Initialise();
}

ImageLoaderFreeImage::~ImageLoaderFreeImage()
{
  FreeImage_DeInitialise();
}

std::shared_ptr<ImageData>
ImageLoaderFreeImage::LoadImpl(const std::string &_filename) const
{
    FREE_IMAGE_FORMAT fifmt = FreeImage_GetFIFFromFilename(_filename.c_str());
    FIBITMAP *bitmap{nullptr};

    if (fifmt == FIF_PNG)
    {
      bitmap = FreeImage_Load(fifmt, _filename.c_str(), PNG_DEFAULT);
    }
    else if (fifmt == FIF_JPEG)
    {
      bitmap = FreeImage_Load(fifmt, _filename.c_str(), JPEG_DEFAULT);
    }
    else if (fifmt == FIF_BMP)
    {
      bitmap = FreeImage_Load(fifmt, _filename.c_str(), BMP_DEFAULT);
    }
    else
    {
      gzerr << "Unknown image format[" << _filename << "]\n";
      return {nullptr};
    }

    auto ret = std::make_shared<ImageData>();
    ret->filename = _filename;
    ret->width = FreeImage_GetWidth(bitmap);
    ret->height = FreeImage_GetHeight(bitmap);

    ret->bpp = FreeImage_GetBPP(bitmap);
    ret->pixel_format = gz::common::Image::UNKNOWN_PIXEL_FORMAT;

    FREE_IMAGE_TYPE type = FreeImage_GetImageType(bitmap);
    unsigned int redMask = FreeImage_GetRedMask(bitmap);

    if (type == FIT_BITMAP)
    {
      if (ret->bpp == 8)
        ret->pixel_format = gz::common::Image::L_INT8;
      else if (ret->bpp == 16)
        ret->pixel_format = gz::common::Image::L_INT16;
      else if (ret->bpp == 24)
        redMask == 0xff0000 ?
              ret->pixel_format = gz::common::Image::RGB_INT8 :
              ret->pixel_format = gz::common::Image::BGR_INT8;
      else if (ret->bpp == 32)
      {
        redMask == 0xff0000 || redMask == 0xff000000 ?
          ret->pixel_format = gz::common::Image::RGBA_INT8 :
          ret->pixel_format = gz::common::Image::BGRA_INT8;
      }
    }
    else if (type == FIT_RGB16)
      ret->pixel_format = gz::common::Image::RGB_INT16;
    else if (type == FIT_RGBF)
      ret->pixel_format = gz::common::Image::RGB_FLOAT32;
    else if (type == FIT_UINT16 || type == FIT_INT16)
      ret->pixel_format = gz::common::Image::L_INT16;


    // Always expand rgb into rgba
    if (ret->pixel_format == gz::common::Image::RGB_INT8 ||
        ret->pixel_format == gz::common::Image::BGR_INT8 ||
        ret->pixel_format == gz::common::Image::RGBA_INT8 ||
        ret->pixel_format == gz::common::Image::BGRA_INT8)
    {
      ret->data = RGBAData(bitmap);
      ret->pixel_format = gz::common::Image::RGBA_INT8;
      ret->bpp = 32;
      ret->pitch = (ret->bpp * ret->width) / 8;
    }

    // Normalize bytes per pixel
    ret->bpp /= 8;

    if (bitmap)
      FreeImage_Unload(bitmap);

    return ret;
  }

void ImageLoaderFreeImage::Save(const std::string &_filename, const ImageData &_image) const
{
}

}  // namespace gz::common
