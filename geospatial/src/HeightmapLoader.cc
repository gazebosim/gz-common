/*
 * Copyright (C) 2025 Open Source Robotics Foundation
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

#include "gz/common/Console.hh"
#include "gz/common/StringUtils.hh"
#include "gz/common/Util.hh"

#include "gz/common/geospatial/Dem.hh"
#include "gz/common/geospatial/HeightmapLoader.hh"
#include "gz/common/geospatial/ImageHeightmap.hh"

using namespace gz;
using namespace common;

//////////////////////////////////////////////////
bool HeightmapLoader::SupportedImageFileExtension(
    const std::string &_filename)
{
  if (_filename.empty())
    return false;

  std::string lowerFullPath = common::lowercase(_filename);
  return common::EndsWith(lowerFullPath, ".png")
       || common::EndsWith(lowerFullPath, ".jpg")
       || common::EndsWith(lowerFullPath, ".jpeg");
}

//////////////////////////////////////////////////
std::unique_ptr<HeightmapData> HeightmapLoader::Load(
    const std::string &_filename,
    const math::SphericalCoordinates &_sphericalCoordinates)
{
  if (_filename.empty())
    return nullptr;

  std::unique_ptr<HeightmapData> data;
  // check if heightmap is an image
  if (HeightmapLoader::SupportedImageFileExtension(_filename))
  {
    auto imgHeightmap = std::make_unique<ImageHeightmap>();
    if (imgHeightmap->Load(_filename) < 0)
    {
      gzerr << "Failed to load heightmap data from ["
             << _filename << "]" << std::endl;

      return nullptr;
    }
    data = std::move(imgHeightmap);
  }
  else
  {
    // try loading as DEM
    auto dem = std::make_unique<Dem>();
    dem->SetSphericalCoordinates(_sphericalCoordinates);
    if (dem->Load(_filename) < 0)
    {
      gzerr << "Failed to load heightmap data from ["
             << _filename << "]" << std::endl;
      return nullptr;
    }
    data = std::move(dem);
  }

  return data;
}
