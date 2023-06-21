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

#ifndef GZ_COMMON_GRAPHICS_IMAGELOADER_HH
#define GZ_COMMON_GRAPHICS_IMAGELOADER_HH

#include <gz/common/Image.hh>

namespace gz::common
{
struct ImageData
{
  std::string filename {""};
  int width {-1};
  int height {-1};
  int numChannels {-1};
  int bpp {-1};
  int pitch {-1};

  gz::common::Image::PixelFormatType pixel_format;
  std::vector<uint8_t> data;

  bool InBounds(int _x, int _y) const {
    return (
      _x >= 0 &&
      _y >= 0 &&
      _x < this->width &&
      _y < this->height);
  }

  const uint8_t* At(size_t _x, size_t _y) const {
    if (!InBounds(_x, _y))
      return nullptr;
    return &this->data[_y * this->pitch + _x * bpp];
  }

};

class ImageLoader
{
  public: virtual ~ImageLoader() = 0;
  public: std::shared_ptr<ImageData> Load(const std::string &_filename) const;
  public: virtual std::shared_ptr<ImageData> LoadImpl(const std::string &_filename) const = 0;
  public: virtual void Save(const std::string &_filename, const ImageData &_image) const = 0;
};

}  // namespace gz::common
#endif  // GZ_COMMON_GRAPHICS_IMAGELOADER_HH
