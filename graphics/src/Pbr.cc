/*
 * Copyright 2020 Open Source Robotics Foundation
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
#include <string>
#include <vector>
#include <gz/math/Vector3.hh>

#include "gz/common/Pbr.hh"

/// \brief Private data for Pbr class
class gz::common::PbrPrivate
{
  /// \brief Workflow type
  public: PbrType type = PbrType::NONE;

  /// \brief Albedo map
  public: std::string albedoMap = "";

  /// \brief Normal map
  public: std::string normalMap = "";

  /// \brief Normal map space
  public: NormalMapSpace normalMapSpace = NormalMapSpace::TANGENT;

  /// \brief Environment map
  public: std::string environmentMap = "";

  /// \brief Ambient occlusion map
  public: std::string ambientOcclusionMap = "";

  /// \brief Roughness map (metal workflow only)
  public: std::string roughnessMap = "";

  /// \brief Metalness map (metal workflow only)
  public: std::string metalnessMap = "";

  /// \brief Emissive map
  public: std::string emissiveMap = "";

  /// \brief Light map
  public: std::string lightMap;

  /// \brief Light map texture coordinate set
  public: unsigned int lightMapUvSet = 0u;

  /// \brief Roughness value (metal workflow only)
  public: double roughness = 0.5;

  /// \brief Metalness value (metal workflow only)
  public: double metalness = 0.0;

  /// \brief Specular map (specular workflow only)
  public: std::string specularMap = "";

  /// \brief Glossiness map (specular workflow only)
  public: std::string glossinessMap = "";

  /// \brief Glossiness value (specular workflow only)
  public: double glossiness = 0.0;
};


using namespace ignition;
using namespace common;

/////////////////////////////////////////////////
Pbr::Pbr()
  : dataPtr(new PbrPrivate)
{
}

/////////////////////////////////////////////////
Pbr::~Pbr()
{
  delete this->dataPtr;
  this->dataPtr = nullptr;
}

//////////////////////////////////////////////////
Pbr::Pbr(const Pbr &_pbr)
  : dataPtr(new PbrPrivate)
{
  *this->dataPtr = *_pbr.dataPtr;
}

/////////////////////////////////////////////////
Pbr &Pbr::operator=(const Pbr &_pbr)
{
  if (!this->dataPtr)
  {
    this->dataPtr = new PbrPrivate;
  }
  *this->dataPtr = *_pbr.dataPtr;
  return *this;
}

/////////////////////////////////////////////////
Pbr &Pbr::operator=(Pbr &&_pbr)
{
  if (this->dataPtr)
  {
    delete this->dataPtr;
  }
  this->dataPtr = _pbr.dataPtr;
  _pbr.dataPtr = nullptr;
  return *this;
}

//////////////////////////////////////////////////
bool Pbr::operator!=(const Pbr &_pbr) const
{
  return !(*this == _pbr);
}

/////////////////////////////////////////////////
bool Pbr::operator==(const Pbr &_pbr) const
{
  return (this->dataPtr->albedoMap == _pbr.dataPtr->albedoMap)
    && (this->dataPtr->normalMap == _pbr.dataPtr->normalMap)
    && (this->dataPtr->metalnessMap == _pbr.dataPtr->metalnessMap)
    && (this->dataPtr->roughnessMap == _pbr.dataPtr->roughnessMap)
    && (this->dataPtr->glossinessMap == _pbr.dataPtr->glossinessMap)
    && (this->dataPtr->environmentMap == _pbr.dataPtr->environmentMap)
    && (this->dataPtr->emissiveMap == _pbr.dataPtr->emissiveMap)
    && (this->dataPtr->ambientOcclusionMap ==
        _pbr.dataPtr->ambientOcclusionMap)
    && (math::equal(
        this->dataPtr->metalness, _pbr.dataPtr->metalness))
    && (math::equal(
        this->dataPtr->roughness, _pbr.dataPtr->roughness))
    && (math::equal(
        this->dataPtr->glossiness, _pbr.dataPtr->glossiness));
}

/////////////////////////////////////////////////
Pbr::Pbr(Pbr &&_pbr) noexcept
{
  this->dataPtr = _pbr.dataPtr;
  _pbr.dataPtr = nullptr;
}

//////////////////////////////////////////////////
std::string Pbr::AlbedoMap() const
{
  return this->dataPtr->albedoMap;
}

//////////////////////////////////////////////////
void Pbr::SetAlbedoMap(const std::string &_map)
{
  this->dataPtr->albedoMap = _map;
}

//////////////////////////////////////////////////
std::string Pbr::NormalMap() const
{
  return this->dataPtr->normalMap;
}

//////////////////////////////////////////////////
NormalMapSpace Pbr::NormalMapType() const
{
  return this->dataPtr->normalMapSpace;
}

//////////////////////////////////////////////////
void Pbr::SetNormalMap(const std::string &_map, NormalMapSpace _space)
{
  this->dataPtr->normalMap = _map;
  this->dataPtr->normalMapSpace = _space;
}

//////////////////////////////////////////////////
std::string Pbr::EnvironmentMap() const
{
  return this->dataPtr->environmentMap;
}

//////////////////////////////////////////////////
void Pbr::SetEnvironmentMap(const std::string &_map)
{
  this->dataPtr->environmentMap = _map;
}

//////////////////////////////////////////////////
std::string Pbr::AmbientOcclusionMap() const
{
  return this->dataPtr->ambientOcclusionMap;
}

//////////////////////////////////////////////////
void Pbr::SetAmbientOcclusionMap(const std::string &_map)
{
  this->dataPtr->ambientOcclusionMap = _map;
}

//////////////////////////////////////////////////
std::string Pbr::RoughnessMap() const
{
  return this->dataPtr->roughnessMap;
}

//////////////////////////////////////////////////
void Pbr::SetRoughnessMap(const std::string &_map)
{
  this->dataPtr->roughnessMap = _map;
}

//////////////////////////////////////////////////
std::string Pbr::MetalnessMap() const
{
  return this->dataPtr->metalnessMap;
}

//////////////////////////////////////////////////
void Pbr::SetMetalnessMap(const std::string &_map)
{
  this->dataPtr->metalnessMap = _map;
}

//////////////////////////////////////////////////
double Pbr::Metalness() const
{
  return this->dataPtr->metalness;
}

//////////////////////////////////////////////////
void Pbr::SetMetalness(double _metalness)
{
  this->dataPtr->metalness = _metalness;
}

//////////////////////////////////////////////////
double Pbr::Roughness() const
{
  return this->dataPtr->roughness;
}

//////////////////////////////////////////////////
void Pbr::SetRoughness(double _roughness)
{
  this->dataPtr->roughness = _roughness;
}

//////////////////////////////////////////////////
std::string Pbr::SpecularMap() const
{
  return this->dataPtr->specularMap;
}

//////////////////////////////////////////////////
void Pbr::SetSpecularMap(const std::string &_map)
{
  this->dataPtr->specularMap = _map;
}

//////////////////////////////////////////////////
std::string Pbr::GlossinessMap() const
{
  return this->dataPtr->glossinessMap;
}

//////////////////////////////////////////////////
void Pbr::SetGlossinessMap(const std::string &_map)
{
  this->dataPtr->glossinessMap = _map;
}

//////////////////////////////////////////////////
void Pbr::SetGlossiness(double _glossiness)
{
  this->dataPtr->glossiness = _glossiness;
}

//////////////////////////////////////////////////
double Pbr::Glossiness() const
{
  return this->dataPtr->glossiness;
}

//////////////////////////////////////////////////
std::string Pbr::EmissiveMap() const
{
  return this->dataPtr->emissiveMap;
}

//////////////////////////////////////////////////
void Pbr::SetEmissiveMap(const std::string &_map)
{
  this->dataPtr->emissiveMap = _map;
}

//////////////////////////////////////////////////
std::string Pbr::LightMap() const
{
  return this->dataPtr->lightMap;
}

//////////////////////////////////////////////////
void Pbr::SetLightMap(const std::string &_map, unsigned int _uvSet)
{
  this->dataPtr->lightMap = _map;
  this->dataPtr->lightMapUvSet = _uvSet;
}

//////////////////////////////////////////////////
unsigned int Pbr::LightMapTexCoordSet() const
{
  return this->dataPtr->lightMapUvSet;
}

//////////////////////////////////////////////////
PbrType Pbr::Type() const
{
  return this->dataPtr->type;
}

//////////////////////////////////////////////////
void Pbr::SetType(PbrType _type)
{
  this->dataPtr->type = _type;
}

