/*
 * Copyright (C) 2016 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/
#ifndef IGNITION_COMMON_IMAGE_HH_
#define IGNITION_COMMON_IMAGE_HH_

#include <memory>
#include <string>
#include <ignition/math/Color.hh>
#include <ignition/common/graphics/Export.hh>
#include <ignition/common/SuppressWarning.hh>

namespace ignition
{
  namespace common
  {
    /// \brief Forward declaration of private data class
    class ImagePrivate;

    /// \brief String names for the pixel formats.
    /// \sa Image::PixelFormat.
    static std::string PixelFormatNames[] =
    {
      "UNKNOWN_PIXEL_FORMAT",
      "L_INT8",
      "L_INT16",
      "RGB_INT8",
      "RGBA_INT8",
      "BGRA_INT8",
      "RGB_INT16",
      "RGB_INT32",
      "BGR_INT8",
      "BGR_INT16",
      "BGR_INT32",
      "R_FLOAT16",
      "RGB_FLOAT16",
      "R_FLOAT32",
      "RGB_FLOAT32",
      "BAYER_RGGB8",
      "BAYER_RGGR8",
      "BAYER_GBRG8",
      "BAYER_GRBG8"
    };

    /// \class Image Image.hh ignition/common/common.hh
    /// \brief Encapsulates an image
    class IGNITION_COMMON_GRAPHICS_VISIBLE Image
    {
      /// \brief Pixel formats enumeration
      public: enum PixelFormatType
              {
                UNKNOWN_PIXEL_FORMAT = 0,
                L_INT8,
                L_INT16,
                RGB_INT8,
                RGBA_INT8,
                BGRA_INT8,
                RGB_INT16,
                RGB_INT32,
                BGR_INT8,
                BGR_INT16,
                BGR_INT32,
                R_FLOAT16,
                RGB_FLOAT16,
                R_FLOAT32,
                RGB_FLOAT32,
                BAYER_RGGB8,
                BAYER_RGGR8,
                BAYER_GBRG8,
                BAYER_GRBG8,
                PIXEL_FORMAT_COUNT
              };


      /// \brief Convert a string to a Image::PixelFormat.
      /// \param[in] _format Pixel format string. \sa Image::PixelFormatNames
      /// \return Image::PixelFormat
      public: static Image::PixelFormatType ConvertPixelFormat(
                  const std::string &_format);

      /// \brief Constructor
      /// \param[in] _filename the path to the image
      public: explicit Image(const std::string &_filename="");

      /// \brief Destructor
      public: virtual ~Image();

      /// \brief Load an image. Return 0 on success
      /// \param[in] _filename the path to the image file
      /// \return 0 when the operation succeeds to open a file or -1 when fails.
      public: int Load(const std::string &_filename);

      /// \brief Save the image in PNG format
      /// \param[in] _filename The name of the saved image
      public: void SavePNG(const std::string &_filename);

      /// \brief Set the image from raw data
      /// \param[in] _data Pointer to the raw image data
      /// \param[in] _width Width in pixels
      /// \param[in] _height Height in pixels
      /// \param[in] _format Pixel format of the provided data
      public: void SetFromData(const unsigned char *_data,
                               unsigned int _width,
                               unsigned int _height,
                               Image::PixelFormatType _format);

      /// \brief Get the image as a data array
      /// \param[out] _data Pointer to a NULL array of char.
      /// \param[out] _count The resulting data array size
      public: void Data(unsigned char **_data,
                        unsigned int &_count) const;

      /// \brief Get only the RGB data from the image. This will drop the
      /// alpha channel if one is present.
      /// \param[out] _data Pointer to a NULL array of char.
      /// \param[out] _count The resulting data array size
      public: void RGBData(unsigned char **_data,
                           unsigned int &_count) const;

      /// \brief Get the width
      /// \return The image width
      public: unsigned int Width() const;

      /// \brief Get the height
      /// \return The image height
      public: unsigned int Height() const;

      /// \brief Get the size of one pixel in bits
      /// \return The BPP of the image
      public: unsigned int BPP() const;

      // \brief Get the size of a row of pixel
      /// \return The pitch of the image
      public: int Pitch() const;

      /// \brief Get the full filename of the image
      /// \return The filename used to load the image
      public: std::string Filename() const;

      /// \brief Get the pixel format
      /// \return PixelFormat
      public: PixelFormatType PixelFormat() const;

      /// \brief Get a pixel color value
      /// \param[in] _x Column location in the image
      /// \param[in] _y Row location in the image
      /// \return The color of the given pixel
      public: math::Color Pixel(const unsigned int _x,
                  const unsigned int _y) const;

      /// \brief Get the average color
      /// \return The average color
      public: math::Color AvgColor();

      /// \brief Get the max color
      /// \return The max color
      public: math::Color MaxColor() const;

      /// \brief Rescale the image
      /// \param[in] _width New image width
      /// \param[in] _height New image height
      public: void Rescale(const int _width, const int _height);

      /// \brief Returns whether this is a valid image
      /// \return true if image has a bitmap
      public: bool Valid() const;

      IGN_COMMON_WARN_IGNORE__DLL_INTERFACE_MISSING
      /// \brief Private data pointer
      private: std::unique_ptr<ImagePrivate> dataPtr;
      IGN_COMMON_WARN_RESUME__DLL_INTERFACE_MISSING
    };
  }
}
#endif
