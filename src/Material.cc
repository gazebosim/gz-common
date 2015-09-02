/*
 * Copyright (C) 2015 Open Source Robotics Foundation
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

#include <boost/filesystem.hpp>

#include "ignition/common/Console.hh"
#include "ignition/common/MaterialPrivate.hh"
#include "ignition/common/Material.hh"

using namespace ignition;
using namespace common;

unsigned int MaterialPrivate::counter = 0;

std::string Material::ShadeModeStr[SHADE_COUNT] = {"FLAT", "GOURAUD",
  "PHONG", "BLINN"};
std::string Material::BlendModeStr[BLEND_COUNT] = {"ADD", "MODULATE",
  "REPLACE"};

//////////////////////////////////////////////////
Material::Material()
  : Material(common::Color(0.4, 0.4, 0.4, 1))
{
}

//////////////////////////////////////////////////
Material::Material(const Color &_clr)
  : dataPtr(new MaterialPrivate)
{
  this->dataPtr->name = "ign-common_material_" +
      std::to_string(this->dataPtr->counter++);
  this->dataPtr->blendMode = REPLACE;
  this->dataPtr->shadeMode = GOURAUD;
  this->dataPtr->transparency = 0;
  this->dataPtr->shininess = 0;
  this->dataPtr->ambient = _clr;
  this->dataPtr->diffuse = _clr;
  this->dataPtr->lighting = false;
  this->dataPtr->dstBlendFactor = this->dataPtr->srcBlendFactor = 1.0;
}

//////////////////////////////////////////////////
Material::~Material()
{
  delete this->dataPtr;
  this->dataPtr = NULL;
}

//////////////////////////////////////////////////
std::string Material::Name() const
{
  return this->dataPtr->name;
}

//////////////////////////////////////////////////
void Material::SetTextureImage(const std::string &_tex)
{
  this->dataPtr->texImage = _tex;
}

//////////////////////////////////////////////////
void Material::SetTextureImage(const std::string &_tex,
                               const std::string &_resourcePath)
{
  this->dataPtr->texImage = _resourcePath + "/" + _tex;

  // If the texture image doesn't exist then try the next most likely path.
  if (!boost::filesystem::exists(this->dataPtr->texImage))
  {
    this->dataPtr->texImage = _resourcePath + "/../materials/textures/" + _tex;
    if (!boost::filesystem::exists(this->dataPtr->texImage))
    {
      ignerr << "Unable to find texture[" << _tex << "] in path["
            << _resourcePath << "]\n";
    }
  }
}

//////////////////////////////////////////////////
std::string Material::TextureImage() const
{
  return this->dataPtr->texImage;
}

//////////////////////////////////////////////////
void Material::SetAmbient(const Color &_clr)
{
  this->dataPtr->ambient = _clr;
}

//////////////////////////////////////////////////
Color Material::Ambient() const
{
  return this->dataPtr->ambient;
}

//////////////////////////////////////////////////
void Material::SetDiffuse(const Color &_clr)
{
  this->dataPtr->diffuse = _clr;
  this->dataPtr->lighting = true;
}

//////////////////////////////////////////////////
Color Material::Diffuse() const
{
  return this->dataPtr->diffuse;
}

//////////////////////////////////////////////////
void Material::SetSpecular(const Color &_clr)
{
  this->dataPtr->specular = _clr;
  this->dataPtr->lighting = true;
}

//////////////////////////////////////////////////
Color Material::Specular() const
{
  return this->dataPtr->specular;
}

//////////////////////////////////////////////////
void Material::SetEmissive(const Color &_clr)
{
  this->dataPtr->emissive = _clr;
}

//////////////////////////////////////////////////
Color Material::Emissive() const
{
  return this->dataPtr->emissive;
}

//////////////////////////////////////////////////
void Material::SetTransparency(const double _t)
{
  this->dataPtr->transparency = std::min(_t, 1.0);
  this->dataPtr->transparency = std::max(this->dataPtr->transparency, 0.0);
  this->dataPtr->lighting = true;
}

//////////////////////////////////////////////////
double Material::Transparency() const
{
  return this->dataPtr->transparency;
}

//////////////////////////////////////////////////
void Material::SetShininess(const double _s)
{
  this->dataPtr->shininess = _s;
  this->dataPtr->lighting = true;
}

//////////////////////////////////////////////////
double Material::Shininess() const
{
  return this->dataPtr->shininess;
}

//////////////////////////////////////////////////
void Material::SetBlendFactors(const double _srcFactor, const double _dstFactor)
{
  this->dataPtr->srcBlendFactor = _srcFactor;
  this->dataPtr->dstBlendFactor = _dstFactor;
}

//////////////////////////////////////////////////
void Material::BlendFactors(double &_srcFactor, double &_dstFactor) const
{
  _srcFactor = this->dataPtr->srcBlendFactor;
  _dstFactor = this->dataPtr->dstBlendFactor;
}

//////////////////////////////////////////////////
void Material::SetBlendMode(const MaterialBlendMode _b)
{
  this->dataPtr->blendMode = _b;
}

//////////////////////////////////////////////////
Material::MaterialBlendMode Material::BlendMode() const
{
  return this->dataPtr->blendMode;
}

//////////////////////////////////////////////////
void Material::SetShadeMode(const MaterialShadeMode _s)
{
  this->dataPtr->shadeMode = _s;
}

//////////////////////////////////////////////////
Material::MaterialShadeMode Material::ShadeMode() const
{
  return this->dataPtr->shadeMode;
}

//////////////////////////////////////////////////
void Material::SetPointSize(const double _size)
{
  this->dataPtr->pointSize = _size;
}

//////////////////////////////////////////////////
double Material::PointSize() const
{
  return this->dataPtr->pointSize;
}
//////////////////////////////////////////////////
void Material::SetDepthWrite(const bool _value)
{
  this->dataPtr->depthWrite = _value;
}

//////////////////////////////////////////////////
bool Material::DepthWrite() const
{
  return this->dataPtr->depthWrite;
}

//////////////////////////////////////////////////
void Material::SetLighting(const bool _value)
{
  this->dataPtr->lighting = _value;
}

//////////////////////////////////////////////////
bool Material::Lighting() const
{
  return this->dataPtr->lighting;
}

//////////////////////////////////////////////////
std::ostream &operator<<(std::ostream &_out, const Material &_m)
{
  _out << "Material:\n";
  _out << "  Name: " << _m.Name() << "\n";
  _out << "  Texture: " << _m.TextureImage() << "\n";
  _out << "  Ambient: " << _m.Ambient() << "\n";
  _out << "  Diffuse: " << _m.Diffuse() << "\n";
  _out << "  Specular: " << _m.Specular() << "\n";
  _out << "  Emissive: " << _m.Emissive() << "\n";
  _out << "  Transparency: " << _m.Transparency() << "\n";
  _out << "  Shininess: " << _m.Shininess() << "\n";
  _out << "  BlendMode: " << Material::BlendModeStr[_m.BlendMode()] << "\n";
  _out << "  ShadeMode: " << Material::ShadeModeStr[_m.ShadeMode()] << "\n";
  _out << "  DepthWrite: " << _m.DepthWrite() << "\n";
  return _out;
}
