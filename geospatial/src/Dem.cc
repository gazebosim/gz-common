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
#include <algorithm>
#include <limits>

#include <gdal_priv.h>
#include <ogr_spatialref.h>

#include "ignition/common/Console.hh"
#include "ignition/common/geospatial/Dem.hh"
#include "ignition/math/SphericalCoordinates.hh"

using namespace ignition;
using namespace common;

class ignition::common::Dem::Implementation
{
  /// \brief A set of associated raster bands.
  public: GDALDataset *dataSet;

  /// \brief A pointer to the band.
  public: GDALRasterBand *band;

  /// \brief Real width of the world in meters.
  public: double worldWidth;

  /// \brief Real height of the world in meters.
  public: double worldHeight;

  /// \brief Terrain's side (after the padding).
  public: unsigned int side;

  /// \brief Minimum elevation in meters.
  public: double minElevation;

  /// \brief Maximum elevation in meters.
  public: double maxElevation;

  /// \brief DEM data converted to be OGRE-compatible.
  public: std::vector<float> demData;

  /// \brief Full filename used to load the dem.
  public: std::string filename;

  /// \brief Whether the DEM will be handled as from non-Earth.
  /// If true, worldWidth & worldHeight = -1
  /// and GeoReference[Origin] can not be used (will return false)
  public: bool isNonEarthDem = false;
};

//////////////////////////////////////////////////
Dem::Dem()
: dataPtr(ignition::utils::MakeImpl<Implementation>())
{
  this->dataPtr->dataSet = nullptr;
  GDALAllRegister();
}

//////////////////////////////////////////////////
Dem::~Dem()
{
  this->dataPtr->demData.clear();

  if (this->dataPtr->dataSet)
    GDALClose(reinterpret_cast<GDALDataset *>(this->dataPtr->dataSet));
}

//////////////////////////////////////////////////
int Dem::Load(const std::string &_filename)
{
  unsigned int width;
  unsigned int height;
  int xSize, ySize;
  double upLeftX, upLeftY, upRightX, upRightY, lowLeftX, lowLeftY;
  ignition::math::Angle upLeftLat, upLeftLong, upRightLat, upRightLong;
  ignition::math::Angle lowLeftLat, lowLeftLong;

  // Sanity check
  std::string fullName = _filename;
  if (!exists(findFilePath(fullName)))
    fullName = common::findFile(_filename);

  this->dataPtr->filename = fullName;

  if (!exists(findFilePath(fullName)))
  {
    ignerr << "Unable to find DEM file[" << _filename << "]." << std::endl;
    return -1;
  }

  this->dataPtr->dataSet = reinterpret_cast<GDALDataset *>(GDALOpen(
    fullName.c_str(), GA_ReadOnly));

  if (this->dataPtr->dataSet == nullptr)
  {
    ignerr << "Unable to open DEM file[" << fullName
           << "]. Format not recognized as a supported dataset." << std::endl;
    return -1;
  }

  int nBands = this->dataPtr->dataSet->GetRasterCount();
  if (nBands != 1)
  {
    ignerr << "Unsupported number of bands in file [" << fullName + "]. Found "
          << nBands << " but only 1 is a valid value." << std::endl;
    return -1;
  }

  // Set the pointer to the band
  this->dataPtr->band = this->dataPtr->dataSet->GetRasterBand(1);

  // Raster width and height
  xSize = this->dataPtr->dataSet->GetRasterXSize();
  ySize = this->dataPtr->dataSet->GetRasterYSize();

  // Corner coordinates
  upLeftX = 0.0;
  upLeftY = 0.0;
  upRightX = xSize;
  upRightY = 0.0;
  lowLeftX = 0.0;
  lowLeftY = ySize;

  // Calculate the georeferenced coordinates of the terrain corners
  if (this->GeoReference(upLeftX, upLeftY, upLeftLat, upLeftLong)
      && this->GeoReference(upRightX, upRightY, upRightLat, upRightLong)
      && this->GeoReference(lowLeftX, lowLeftY, lowLeftLat, lowLeftLong))
  {
    // If successful, set the world width and height
    this->dataPtr->worldWidth =
       math::SphericalCoordinates::Distance(upLeftLat, upLeftLong,
                                              upRightLat, upRightLong);
    this->dataPtr->worldHeight =
       math::SphericalCoordinates::Distance(upLeftLat, upLeftLong,
                                              lowLeftLat, lowLeftLong);
  }
  // Assume non-Earth DEM (e.g., moon)
  else
  {
    ignwarn << "Failed to automatically compute DEM size. "
            << "Assuming non-Earth DEM. "
            << std::endl;

    this->dataPtr->worldWidth = this->dataPtr->worldHeight = -1;
    this->dataPtr->isNonEarthDem = true;
  }

  // Set the terrain's side (the terrain will be squared after the padding)
  if (ignition::math::isPowerOfTwo(ySize - 1))
    height = ySize;
  else
    height = ignition::math::roundUpPowerOfTwo(ySize) + 1;

  if (ignition::math::isPowerOfTwo(xSize - 1))
    width = xSize;
  else
    width = ignition::math::roundUpPowerOfTwo(xSize) + 1;

  this->dataPtr->side = std::max(width, height);

  // Preload the DEM's data
  if (this->LoadData() != 0)
    return -1;

  // Check for nodata value in dem data. This is used when computing the
  // min elevation. If nodata value is not defined, we assume it will be one
  // of the commonly used values such as -9999, -32768, etc.
  // See https://desktop.arcgis.com/en/arcmap/10.8/manage-data/raster-and-images/nodata-in-raster-datasets.htm
  // For simplicity, we will treat values <= -9999 as nodata values and
  // ignore them when computing the min elevation.
  int validNoData = 0;
  const double defaultNoDataValue = -9999;
  double noDataValue = this->dataPtr->band->GetNoDataValue(&validNoData);
  if (validNoData <= 0)
    noDataValue = defaultNoDataValue;

  double min = ignition::math::MAX_D;
  double max = -ignition::math::MAX_D;
  for (auto d : this->dataPtr->demData)
  {
    if (d > noDataValue)
    {
      if (d < min)
        min = d;
      if (d > max)
        max = d;
    }
  }
  if (ignition::math::equal(min, ignition::math::MAX_D) ||
      ignition::math::equal(max, -ignition::math::MAX_D))
  {
    ignwarn << "DEM is composed of 'nodata' values!" << std::endl;
  }

  this->dataPtr->minElevation = min;
  this->dataPtr->maxElevation = max;
  return 0;
}

//////////////////////////////////////////////////
double Dem::Elevation(double _x, double _y)
{
  if (_x >= this->Width() || _y >= this->Height())
  {
    ignerr << "Illegal coordinates. You are asking for the elevation in ("
           << _x << "," << _y << ") but the terrain is ["
           << this->Width() << " x " << this->Height() << "]" << std::endl;
    return std::numeric_limits<double>::infinity();
  }

  auto idx = static_cast<unsigned int>(_y) * this->Width()
      + static_cast<unsigned int>(_x);
  return this->dataPtr->demData.at(idx);
}

//////////////////////////////////////////////////
float Dem::MinElevation() const
{
  return this->dataPtr->minElevation;
}

//////////////////////////////////////////////////
float Dem::MaxElevation() const
{
  return this->dataPtr->maxElevation;
}

//////////////////////////////////////////////////
bool Dem::GeoReference(double _x, double _y,
    ignition::math::Angle &_latitude, ignition::math::Angle &_longitude) const
{
  if (this->dataPtr->isNonEarthDem)
  {
    ignerr << "Can not retrieve WGS84 coordinates from non-Earth DEM."
            << std::endl;
    return false;
  }

  double geoTransf[6];
  if (this->dataPtr->dataSet->GetGeoTransform(geoTransf) == CE_None)
  {
    OGRSpatialReference sourceCs;
    OGRSpatialReference targetCs;
    OGRCoordinateTransformation *cT;
    double xGeoDeg, yGeoDeg;

    // Transform the terrain's coordinate system to WGS84
    const char *importString
        = strdup(this->dataPtr->dataSet->GetProjectionRef());
    sourceCs.importFromWkt(&importString);
    targetCs.SetWellKnownGeogCS("WGS84");
    cT = OGRCreateCoordinateTransformation(&sourceCs, &targetCs);
    if (nullptr == cT)
    {
      ignerr << "Unable to transform terrain coordinate system to WGS84 for "
             << "coordinates (" << _x << "," << _y << ")" << std::endl;
      OCTDestroyCoordinateTransformation(cT);
      return false;
    }

    xGeoDeg = geoTransf[0] + _x * geoTransf[1] + _y * geoTransf[2];
    yGeoDeg = geoTransf[3] + _x * geoTransf[4] + _y * geoTransf[5];

    cT->Transform(1, &xGeoDeg, &yGeoDeg);

    _latitude.SetDegree(yGeoDeg);
    _longitude.SetDegree(xGeoDeg);

    OCTDestroyCoordinateTransformation(cT);
  }
  else
  {
    ignerr << "Unable to obtain the georeferenced values for coordinates ("
           << _x << "," << _y << ")" << std::endl;
    return false;
  }
  return true;
}

//////////////////////////////////////////////////
bool Dem::GeoReferenceOrigin(ignition::math::Angle &_latitude,
    ignition::math::Angle &_longitude) const
{
  return this->GeoReference(0, 0, _latitude, _longitude);
}

//////////////////////////////////////////////////
unsigned int Dem::Height() const
{
  return this->dataPtr->side;
}

//////////////////////////////////////////////////
unsigned int Dem::Width() const
{
  return this->dataPtr->side;
}

//////////////////////////////////////////////////
double Dem::WorldWidth() const
{
  if (this->dataPtr->isNonEarthDem)
  {
    ignwarn << "Unable to determine world width of non-Earth DEM."
            << std::endl;
  }
  return this->dataPtr->worldWidth;
}

//////////////////////////////////////////////////
double Dem::WorldHeight() const
{
  if (this->dataPtr->isNonEarthDem)
  {
    ignwarn << "Unable to determine world height of non-Earth DEM."
            << std::endl;
  }
  return this->dataPtr->worldHeight;
}

//////////////////////////////////////////////////
void Dem::FillHeightMap(int _subSampling, unsigned int _vertSize,
    const ignition::math::Vector3d &_size,
    const ignition::math::Vector3d &_scale,
    bool _flipY, std::vector<float> &_heights) const
{
  if (_subSampling <= 0)
  {
    ignerr << "Illegal subsampling value (" << _subSampling << ")\n";
    return;
  }

  // Resize the vector to match the size of the vertices.
  _heights.resize(_vertSize * _vertSize);

  // Iterate over all the vertices
  for (unsigned int y = 0; y < _vertSize; ++y)
  {
    double yf = y / static_cast<double>(_subSampling);
    unsigned int y1 = static_cast<unsigned int>(floor(yf));
    unsigned int y2 = static_cast<unsigned int>(ceil(yf));
    if (y2 >= this->dataPtr->side)
      y2 = this->dataPtr->side - 1;
    double dy = yf - y1;

    for (unsigned int x = 0; x < _vertSize; ++x)
    {
      double xf = x / static_cast<double>(_subSampling);
      unsigned int x1 = static_cast<unsigned int>(floor(xf));
      unsigned int x2 = static_cast<unsigned int>(ceil(xf));
      if (x2 >= this->dataPtr->side)
        x2 = this->dataPtr->side - 1;
      double dx = xf - x1;

      double px1 = this->dataPtr->demData[y1 * this->dataPtr->side + x1];
      double px2 = this->dataPtr->demData[y1 * this->dataPtr->side + x2];
      float h1 = (px1 - ((px1 - px2) * dx));

      double px3 = this->dataPtr->demData[y2 * this->dataPtr->side + x1];
      double px4 = this->dataPtr->demData[y2 * this->dataPtr->side + x2];
      float h2 = (px3 - ((px3 - px4) * dx));

      float h = this->dataPtr->minElevation +
          (h1 - ((h1 - h2) * dy) - this->dataPtr->minElevation) * _scale.Z();

      // Invert pixel definition so 1=ground, 0=full height,
      // if the terrain size has a negative z component
      // this is mainly for backward compatibility
      if (_size.Z() < 0)
        h *= -1;

      // Convert to minElevation if a NODATA value is found
      if (_size.Z() >= 0 && h < this->dataPtr->minElevation)
        h = this->dataPtr->minElevation;

      // Store the height for future use
      if (!_flipY)
        _heights[y * _vertSize + x] = h;
      else
        _heights[(_vertSize - y - 1) * _vertSize + x] = h;
    }
  }
}

//////////////////////////////////////////////////
int Dem::LoadData()
{
    unsigned int destWidth;
    unsigned int destHeight;
    unsigned int nXSize = this->dataPtr->dataSet->GetRasterXSize();
    unsigned int nYSize = this->dataPtr->dataSet->GetRasterYSize();
    float ratio;
    std::vector<float> buffer;

    if (nXSize == 0 || nYSize == 0)
    {
      ignerr << "Illegal size loading a DEM file (" << nXSize << ","
            << nYSize << ")\n";
      return -1;
    }

    // Scale the terrain keeping the same ratio between width and height
    if (nXSize > nYSize)
    {
      ratio = static_cast<float>(nXSize) / static_cast<float>(nYSize);
      destWidth = this->dataPtr->side;
      // The decimal part is discarted for interpret the result as pixels
      float h = static_cast<float>(destWidth) / static_cast<float>(ratio);
      destHeight = static_cast<unsigned int>(h);
    }
    else
    {
      ratio = static_cast<float>(nYSize) / static_cast<float>(nXSize);
      destHeight = this->dataPtr->side;
      // The decimal part is discarted for interpret the result as pixels
      float w = static_cast<float>(destHeight) / static_cast<float>(ratio);
      destWidth = static_cast<unsigned int>(w);
    }

    // Read the whole raster data and convert it to a GDT_Float32 array.
    // In this step the DEM is scaled to destWidth x destHeight
    buffer.resize(destWidth * destHeight);
    if (this->dataPtr->band->RasterIO(GF_Read, 0, 0, nXSize, nYSize, &buffer[0],
                         destWidth, destHeight, GDT_Float32, 0, 0) != CE_None)
    {
      ignerr << "Failure calling RasterIO while loading a DEM file\n";
      return -1;
    }

    // Copy and align 'buffer' into the target vector. The destination vector is
    // initialized to 0, so all the points not contained in 'buffer' will be
    // extra padding
    this->dataPtr->demData.resize(this->Width() * this->Height());
    for (unsigned int y = 0; y < destHeight; ++y)
    {
        std::copy(&buffer[destWidth * y], &buffer[destWidth * y] + destWidth,
                  this->dataPtr->demData.begin() + this->Width() * y);
    }
    buffer.clear();

    return 0;
}

//////////////////////////////////////////////////
std::string Dem::Filename() const
{
  return this->dataPtr->filename;
}
