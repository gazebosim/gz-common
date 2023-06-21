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

#include "ImageLoaderSTB.hh"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <gz/common/Console.hh>


namespace gz::common
{

ImageLoaderSTB::ImageLoaderSTB()
{
  stbi_convert_iphone_png_to_rgb( 1 );
  stbi_set_unpremultiply_on_load( 1 );
}

ImageLoaderSTB::~ImageLoaderSTB() = default;

std::shared_ptr<ImageData>
ImageLoaderSTB::LoadImpl_8bit(const std::string &_filename) const
{
  int width, height, components;
  stbi_uc* data = stbi_load(_filename.c_str(), &width, &height, &components, 0);

  if (data == nullptr)
  {
    gzerr << "Failed to open file [" << _filename << "]: " << stbi_failure_reason() << std::endl;
    return nullptr;
  }

  auto ret = std::make_shared<ImageData>();
  ret->filename = _filename;
  ret->width = width;
  ret->height = height;

  switch(components)
  {
    case 1:
      ret->pixel_format = Image::L_INT8;
      ret->bpp = 8;
      break;
    case 3:
      // Copy into RGBA regardless
      ret->pixel_format = Image::RGB_INT8;
      ret->bpp = 24;
      break;
    case 4:
      ret->pixel_format = Image::RGBA_INT8;
      ret->bpp = 32;
      break;
    case 2:
    default:
      gzerr << "Unsupported number of image components: " << components << std::endl;
      stbi_image_free(data);
      return {};
  }

  ret->pitch = ret->bpp / 8 * ret->width;
  ret->data.resize(ret->pitch * ret->height);

  for (int row_idx = 0; row_idx < ret->height; ++row_idx)
  {
    const uint8_t* srcRow = &data[row_idx * ret->pitch];
    uint8_t* destRow = &ret->data[row_idx * ret->pitch];

    std::cout << row_idx << std::endl;

    for (int col_idx = 0; col_idx < ret->width; ++col_idx)
    {
      const uint8_t* srcPixel = &srcRow[col_idx * ret->bpp / 8];
      uint8_t* destPixel = &destRow[col_idx * ret->bpp / 8];

      std::cout << row_idx * ret->pitch + (col_idx * ret->bpp) / 8 << std::endl;

      if (components == 1)
      {
        *destPixel++ = *srcPixel++;
      }
      else if (components == 3)
      {
        *destPixel++ = *srcPixel++;
        *destPixel++ = *srcPixel++;
        *destPixel++ = *srcPixel++;
      }
      else if (components == 4)
      {
        *destPixel++ = *srcPixel++;
        *destPixel++ = *srcPixel++;
        *destPixel++ = *srcPixel++;
        *destPixel++ = *srcPixel++;
      }
    }
  }

  stbi_image_free(data);
  return ret;
}

std::shared_ptr<ImageData>
ImageLoaderSTB::LoadImpl_16bit(const std::string &_filename) const
{
  int width, height, components;
  stbi_us* data = stbi_load_16(_filename.c_str(), &width, &height, &components, 0);

  if (data == nullptr)
  {
    gzerr << "Failed to open file [" << _filename << "]: " << stbi_failure_reason() << std::endl;
    return nullptr;
  }

  auto ret = std::make_shared<ImageData>();
  ret->filename = _filename;
  ret->width = width;
  ret->height = height;

  /*
  switch(components)
  {
    case 1:
      ret->bpp = 16;
      ret->pixel_format = Image::L_INT16;
      break;
    case 3:
      ret->bpp = 48;
      ret->pixel_format = Image::RGB_INT16;
      break;
    case 2:
    case 4:
    default:
      gzerr << "Unsupported number of image components: " << components << std::endl;
      stbi_image_free(data);
      return {};
  }
  ret->pitch = (ret->bpp * ret->width)/8;
    */

  stbi_image_free(data);
  return ret;
}

std::shared_ptr<ImageData>
ImageLoaderSTB::LoadImpl_float(const std::string &_filename) const
{
  int width, height, components;
  float* data = stbi_loadf(_filename.c_str(), &width, &height, &components, 0);

  auto ret = std::make_shared<ImageData>();
  ret->filename = _filename;
  ret->width = width;
  ret->height = height;

  /*
  switch(components)
  {
    case 1:
      ret->bpp = 32;
      ret->pixel_format = Image::R_FLOAT32;
      break;
    case 3:
      ret->bpp = 86;
      ret->pixel_format = Image::RGB_FLOAT32;
      break;
    case 2:
    case 4:
    default:
      gzerr << "Unsupported number of image components: " << components << std::endl;
      stbi_image_free(data);
      return {};
  }
  ret->pitch = (ret->bpp * ret->width)/8;
    */


  stbi_image_free(data);
  return ret;
}


std::shared_ptr<ImageData>
ImageLoaderSTB::LoadImpl(const std::string &_filename) const
{
  if(stbi_is_hdr(_filename.c_str()))
  {
    return LoadImpl_float(_filename);
  }
  else if (stbi_is_16_bit(_filename.c_str()))
  {
    return LoadImpl_16bit(_filename);
  }
  else
  {
    return LoadImpl_8bit(_filename);
  }
}

void ImageLoaderSTB::Save(const std::string &_filename, const ImageData &_image) const
{

}

}  // namespace gz::common
