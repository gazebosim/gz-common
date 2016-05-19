/*
 * Copyright (C) 2012-2016 Open Source Robotics Foundation
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
#include <algorithm>

#include "ignition/common/Material.hh"
#include "ignition/common/Console.hh"

using namespace ignition;
using namespace common;

unsigned int Material::counter = 0;

IGN_ENUM(Material::ShadeMode,
    Material::SHADE_MODE_BEGIN, Material::SHADE_MODE_END,
    "FLAT", "GOURAUD", "PHONG", "BLINN")

IGN_ENUM(Material::BlendMode,
    Material::BLEND_MODE_BEGIN, Material::BLEND_MODE_END,
    "ADD", "MODULATE", "REPLACE")

//////////////////////////////////////////////////
Material::Material()
{
  this->name = "ignition_material_" + std::to_string(counter++);
  this->blendMode = REPLACE;
  this->shadeMode = GOURAUD;
  this->transparency = 0;
  this->shininess = 0;
  this->ambient.Set(0.4, 0.4, 0.4, 1);
  this->diffuse.Set(0.5, 0.5, 0.5, 1);
  this->specular.Set(0, 0, 0, 1);
  this->lighting = false;
  this->dstBlendFactor = this->srcBlendFactor = 1.0;
}

//////////////////////////////////////////////////
Material::Material(const Color &_clr)
{
  this->name = "ignition_material_" + std::to_string(counter++);
  this->blendMode = REPLACE;
  this->shadeMode = GOURAUD;
  this->transparency = 0;
  this->shininess = 0;
  this->ambient = _clr;
  this->diffuse = _clr;
  this->lighting = false;
}

//////////////////////////////////////////////////
Material::~Material()
{
}

//////////////////////////////////////////////////
std::string Material::Name() const
{
  return this->name;
}

//////////////////////////////////////////////////
void Material::SetTextureImage(const std::string &_tex)
{
  this->texImage = _tex;
}

//////////////////////////////////////////////////
void Material::SetTextureImage(const std::string &_tex,
                               const std::string &_resourcePath)
{
  this->texImage = _resourcePath + "/" + _tex;

  // If the texture image doesn't exist then try the next most likely path.
  if (!exists(this->texImage))
  {
    this->texImage = _resourcePath + "/../materials/textures/" + _tex;
    if (!exists(this->texImage))
    {
      ignerr << "Unable to find texture[" << _tex << "] in path["
            << _resourcePath << "]\n";
    }
  }
}

//////////////////////////////////////////////////
std::string Material::TextureImage() const
{
  return this->texImage;
}

//////////////////////////////////////////////////
void Material::SetAmbient(const Color &_clr)
{
  this->ambient = _clr;
}

//////////////////////////////////////////////////
Color Material::Ambient() const
{
  return this->ambient;
}

//////////////////////////////////////////////////
void Material::SetDiffuse(const Color &_clr)
{
  this->diffuse = _clr;
  this->lighting = true;
}

//////////////////////////////////////////////////
Color Material::Diffuse() const
{
  return this->diffuse;
}

//////////////////////////////////////////////////
void Material::SetSpecular(const Color &_clr)
{
  this->specular = _clr;
  this->lighting = true;
}

//////////////////////////////////////////////////
Color Material::Specular() const
{
  return this->specular;
}

//////////////////////////////////////////////////
void Material::SetEmissive(const Color &_clr)
{
  this->emissive = _clr;
}

//////////////////////////////////////////////////
Color Material::Emissive() const
{
  return this->emissive;
}

//////////////////////////////////////////////////
void Material::SetTransparency(double _t)
{
  this->transparency = std::min(_t, 1.0);
  this->transparency = std::max(this->transparency, 0.0);
  this->lighting = true;
}

//////////////////////////////////////////////////
double Material::Transparency() const
{
  return this->transparency;
}

//////////////////////////////////////////////////
void Material::SetShininess(double _s)
{
  this->shininess = _s;
  this->lighting = true;
}

//////////////////////////////////////////////////
double Material::Shininess() const
{
  return this->shininess;
}

//////////////////////////////////////////////////
void Material::SetBlendFactors(double _srcFactor, double _dstFactor)
{
  this->srcBlendFactor = _srcFactor;
  this->dstBlendFactor = _dstFactor;
}

//////////////////////////////////////////////////
void Material::BlendFactors(double &_srcFactor, double &_dstFactor)
{
  _srcFactor = this->srcBlendFactor;
  _dstFactor = this->dstBlendFactor;
}


//////////////////////////////////////////////////
void Material::SetBlend(Material::BlendMode _b)
{
  this->blendMode = _b;
}

//////////////////////////////////////////////////
Material::BlendMode Material::Blend() const
{
  return this->blendMode;
}

//////////////////////////////////////////////////
void Material::SetShade(Material::ShadeMode _s)
{
  this->shadeMode = _s;
}

//////////////////////////////////////////////////
Material::ShadeMode Material::Shade() const
{
  return this->shadeMode;
}

//////////////////////////////////////////////////
void Material::SetPointSize(double _size)
{
  this->pointSize = _size;
}

//////////////////////////////////////////////////
double Material::PointSize() const
{
  return this->pointSize;
}
//////////////////////////////////////////////////
void Material::SetDepthWrite(bool _value)
{
  this->depthWrite = _value;
}

//////////////////////////////////////////////////
bool Material::DepthWrite() const
{
  return this->depthWrite;
}

//////////////////////////////////////////////////
void Material::SetLighting(bool _value)
{
  this->lighting = _value;
}

//////////////////////////////////////////////////
bool Material::Lighting() const
{
  return this->lighting;
}
