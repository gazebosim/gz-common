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
      public: int Load(const std::string &_filename="");

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
    };
  }
}
#endif
