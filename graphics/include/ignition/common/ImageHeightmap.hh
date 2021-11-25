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
#ifndef IGNITION_COMMON_IMAGEHEIGHTMAPDATA_HH_
#define IGNITION_COMMON_IMAGEHEIGHTMAPDATA_HH_

#include <string>
#include <vector>
#include <ignition/math/Vector3.hh>

#include <ignition/common/graphics/Export.hh>
#include <ignition/common/HeightmapData.hh>
#include <ignition/common/Image.hh>

namespace ignition
{
  namespace common
  {
    /// \brief Encapsulates an image that will be interpreted as a heightmap.
    class IGNITION_COMMON_GRAPHICS_VISIBLE ImageHeightmap
      : public ignition::common::HeightmapData
    {
      /// \brief Constructor
      /// \param[in] _filename the path to the image
      public: ImageHeightmap();

      /// \brief Load an image file as a heightmap.
      /// \param[in] _filename the path to the image file.
      /// \return True when the operation succeeds to open a file.
      public: int Load(const std::string &_filename = "");

      // Documentation inherited.
      public: void FillHeightMap(int _subSampling, unsigned int _vertSize,
          const ignition::math::Vector3d &_size,
          const ignition::math::Vector3d &_scale, bool _flipY,
          std::vector<float> &_heights);

      /// \brief Get the full filename of the image
      /// \return The filename used to load the image
      public: std::string Filename() const;

      // Documentation inherited.
      public: unsigned int Height() const;

      // Documentation inherited.
      public: unsigned int Width() const;

      // Documentation inherited.
      public: float MaxElevation() const;

      /// \brief Image containing the heightmap data.
      private: ignition::common::Image img;

      /// \brief Get Heightmap heights given the image
      private: template <typename T>
      void getHeights(T data, const double& maxPixelValue,
        const int& imgHeight, const int& imgWidth,
        const unsigned int& pitch, const unsigned int& bpp,
        const int& _subSampling, unsigned int& _vertSize,
        const ignition::math::Vector3d &_size,
        const ignition::math::Vector3d &_scale,
        const bool& _flipY, std::vector<float> &_heights)
      {
        // Iterate over all the vertices
        for (unsigned int y = 0; y < _vertSize; ++y)
        {
          // yf ranges between 0 and 4
          double yf = y / static_cast<double>(_subSampling);
          int y1 = static_cast<int>(std::floor(yf));
          int y2 = static_cast<int>(std::ceil(yf));
          if (y2 >= imgHeight)
            y2 = imgHeight-1;
          double dy = yf - y1;

          for (unsigned int x = 0; x < _vertSize; ++x)
          {
            double xf = x / static_cast<double>(_subSampling);
            int x1 = static_cast<int>(std::floor(xf));
            int x2 = static_cast<int>(std::ceil(xf));
            if (x2 >= imgWidth)
              x2 = imgWidth-1;
            double dx = xf - x1;

            double px1 = static_cast<int>(
              data[y1 * pitch / bpp + x1]) / maxPixelValue;
            double px2 = static_cast<int>(
              data[y1 * pitch / bpp + x2]) / maxPixelValue;
            float h1 = (px1 - ((px1 - px2) * dx));

            double px3 = static_cast<int>(
              data[y2 * pitch / bpp + x1]) / maxPixelValue;
            double px4 = static_cast<int>(
              data[y2 * pitch / bpp + x2]) / maxPixelValue;
            float h2 = (px3 - ((px3 - px4) * dx));

            float h = (h1 - ((h1 - h2) * dy)) * _scale.Z();

            // invert pixel definition so 1=ground, 0=full height,
            //   if the terrain size has a negative z component
            //   this is mainly for backward compatibility
            if (_size.Z() < 0)
              h = 1.0 - h;

            // Store the height for future use
            if (!_flipY)
              _heights[y * _vertSize + x] = h;
            else
              _heights[(_vertSize - y - 1) * _vertSize + x] = h;
          }
        }
        delete [] data;
      }
    };
  }
}
#endif
