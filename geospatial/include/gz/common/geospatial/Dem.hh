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
#ifndef GZ_COMMON_GEOSPATIAL_DEM_HH_
#define GZ_COMMON_GEOSPATIAL_DEM_HH_

#include <memory>
#include <string>
#include <vector>

#include <gz/math/Vector3.hh>
#include <gz/math/Angle.hh>

#include <gz/common/geospatial/Export.hh>
#include <gz/common/geospatial/HeightmapData.hh>

#include <gz/utils/ImplPtr.hh>


namespace gz
{
  namespace common
  {
    /// \class DEM DEM.hh common/common.hh
    /// \brief Encapsulates a DEM (Digital Elevation Model) file.
    class GZ_COMMON_GEOSPATIAL_VISIBLE Dem : public HeightmapData
    {
      /// \brief Constructor.
      public: Dem();

      /// \brief Destructor.
      public: virtual ~Dem();

      /// \brief Sets the spherical coordinates reference object.
      public: void SetSphericalCoordinates(const math::SphericalCoordinates);

      /// \brief Load a DEM file.
      /// \param[in] _filename the path to the terrain file.
      /// \return 0 when the operation succeeds to open a file.
      public: int Load(const std::string &_filename = "");

      /// \brief Indicate that this is a non Earth DEM.
      /// \param[in] _isNonEarthDem Should be true if this is a
      /// non earth DEM, otherwise false.
      public: void SetNonEarthDEM(bool _isNonEarthDem);

      /// \brief Check if the loaded DEM is not from the Earth.
      /// \return True if the loaded DEM is from the Earth, otherwise
      /// returns False.
      public: bool GetNonEarthDEM();

      /// \brief Get the elevation of a terrain's point in meters.
      /// \param[in] _x X coordinate of the terrain.
      /// \param[in] _y Y coordinate of the terrain.
      /// \return Terrain's elevation at (x,y) in meters or infinity if illegal
      /// coordinates were provided.
      public: double Elevation(double _x, double _y);

      /// \brief Get the terrain's minimum elevation in meters.
      /// \return The minimum elevation (meters).
      public: float MinElevation() const override;

      /// \brief Get the terrain's maximum elevation in meters.
      /// \return The maximum elevation (meters).
      public: float MaxElevation() const override;

      /// \brief Get the georeferenced coordinates (lat, long) of the terrain's
      /// origin in WGS84.
      /// \param[out] _latitude Georeferenced latitude.
      /// \param[out] _longitude Georeferenced longitude.
      /// \return True if able to retrieve origin coordinates. False otherwise.
      public: bool GeoReferenceOrigin(gz::math::Angle &_latitude,
                  gz::math::Angle &_longitude) const;

      /// \brief Get the terrain's height. Due to the Ogre constrains, this
      /// value will be a power of two plus one. The value returned might be
      /// different that the original DEM height because Data() adds the
      /// padding if necessary.
      /// \return The terrain's height (points) satisfying the ogre constrains
      /// (squared terrain with a height value that must be a power of two plus
      /// one).
      public: unsigned int Height() const override;

      /// \brief Get the terrain's width. Due to the Ogre constrains, this
      /// value will be a power of two plus one. The value returned might be
      /// different that the original DEM width because GetData() adds the
      /// padding if necessary.
      /// \return The terrain's width (points) satisfying the ogre constrains
      /// (squared terrain with a width value that must be a power of two plus
      /// one).
      public: unsigned int Width() const override;

      /// \brief Get the real world width in meters.
      /// \return Terrain's real world width in meters.
      public: double WorldWidth() const;

      /// \brief Get the real world height in meters.
      /// \return Terrain's real world height in meters.
      public: double WorldHeight() const;

      /// \brief Create a lookup table of the terrain's height.
      /// \param[in] _subsampling Multiplier used to increase the resolution.
      /// Ex: A subsampling of 2 in a terrain of 129x129 means that the height
      /// vector will be 257 * 257.
      /// \param[in] _vertSize Number of points per row.
      /// \param[in] _size Real dimmensions of the terrain in meters.
      /// \param[in] _scale Vector3 used to scale the height.
      /// \param[in] _flipY If true, it inverts the order in which the vector
      /// is filled.
      /// \param[out] _heights Vector containing the terrain heights.
      public: void FillHeightMap(const int _subSampling,
                  const unsigned int _vertSize,
                  const gz::math::Vector3d &_size,
                  const gz::math::Vector3d &_scale,
                  const bool _flipY,
                  std::vector<float> &_heights) const override;

      /// \brief Get the georeferenced coordinates (lat, long) of a terrain's
      /// pixel in WGS84.
      /// \param[in] _x X coordinate of the terrain.
      /// \param[in] _y Y coordinate of the terrain.
      /// \param[out] _latitude Georeferenced latitude.
      /// \param[out] _longitude Georeferenced longitude.
      /// \return True if able to retrieve coordinates. False otherwise.
      private: bool GeoReference(double _x, double _y,
                                 gz::math::Angle &_latitude,
                                 gz::math::Angle &_longitude) const;

      /// \brief Get the terrain file as a data array. Due to the Ogre
      /// constrains, the data might be stored in a bigger vector representing
      /// a squared terrain with padding.
      /// \return 0 when the operation succeeds to open a file.
      private: int LoadData();

      // Documentation inherited.
      public: std::string Filename() const override;

      /// internal
      /// \brief Pointer to the private data.
      GZ_UTILS_IMPL_PTR(dataPtr)
    };
  }
}
#endif
