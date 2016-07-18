/*
 * Copyright (C) 2016 Open Source Robotics Foundation
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


IGN_ENUM(Material::ShadeMode,
    Material::SHADE_MODE_BEGIN, Material::SHADE_MODE_END,
    "FLAT", "GOURAUD", "PHONG", "BLINN")

IGN_ENUM(Material::BlendMode,
    Material::BLEND_MODE_BEGIN, Material::BLEND_MODE_END,
    "ADD", "MODULATE", "REPLACE")

/// \brief Private data for Material
class ignition::common::MaterialPrivate
{
  /// \brief the name of the material
  public: std::string name;

  /// \brief the texture image file name
  public: std::string texImage;

  /// \brief the ambient light color
  public: Color ambient;

  /// \brief the diffuse light color
  public: Color diffuse;

  /// \brief the specular light color
  public: Color specular;

  /// \brief the emissive light color
  public: Color emissive;

  /// \brief transparency value in the range 0 to 1
  public: double transparency;

  /// \brief shininess value (0 to 1)
  public: double shininess;

  /// \brief point size
  public: double pointSize;

  /// \brief blend mode
  public: Material::BlendMode blendMode;

  /// \brief the shade mode
  public: Material::ShadeMode shadeMode;

  /// \brief the total number of instantiated Material instances
  public: static unsigned int counter;

  /// \brief flag to perform depth buffer write
  public: bool depthWrite;

  /// \brief flag to indicate if lighting is enabled for this material.
  public: bool lighting;

  /// \brief source blend factor
  public: double srcBlendFactor;

  /// \brief destination blend factor
  public: double dstBlendFactor;
};

unsigned int MaterialPrivate::counter = 0;

//////////////////////////////////////////////////
Material::Material()
: dataPtr(new MaterialPrivate)
{
  this->dataPtr->name = "ignition_material_" +
    std::to_string(this->dataPtr->counter++);
  this->dataPtr->blendMode = REPLACE;
  this->dataPtr->shadeMode = GOURAUD;
  this->dataPtr->transparency = 0;
  this->dataPtr->shininess = 0;
  this->dataPtr->ambient.Set(0.4, 0.4, 0.4, 1);
  this->dataPtr->diffuse.Set(0.5, 0.5, 0.5, 1);
  this->dataPtr->specular.Set(0, 0, 0, 1);
  this->dataPtr->lighting = false;
  this->dataPtr->dstBlendFactor = this->dataPtr->srcBlendFactor = 1.0;
}

//////////////////////////////////////////////////
Material::Material(const Color &_clr)
: dataPtr(new MaterialPrivate)
{
  this->dataPtr->name = "ignition_material_" +
    std::to_string(this->dataPtr->counter++);
  this->dataPtr->blendMode = REPLACE;
  this->dataPtr->shadeMode = GOURAUD;
  this->dataPtr->transparency = 0;
  this->dataPtr->shininess = 0;
  this->dataPtr->ambient = _clr;
  this->dataPtr->diffuse = _clr;
  this->dataPtr->lighting = false;
}

//////////////////////////////////////////////////
Material::~Material()
{
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
  if (!exists(this->dataPtr->texImage))
  {
    this->dataPtr->texImage = _resourcePath + "/../materials/textures/" + _tex;
    if (!exists(this->dataPtr->texImage))
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
void Material::SetTransparency(double _t)
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
void Material::SetShininess(double _s)
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
void Material::SetBlendFactors(double _srcFactor, double _dstFactor)
{
  this->dataPtr->srcBlendFactor = _srcFactor;
  this->dataPtr->dstBlendFactor = _dstFactor;
}

//////////////////////////////////////////////////
void Material::BlendFactors(double &_srcFactor, double &_dstFactor)
{
  _srcFactor = this->dataPtr->srcBlendFactor;
  _dstFactor = this->dataPtr->dstBlendFactor;
}

//////////////////////////////////////////////////
void Material::SetBlend(Material::BlendMode _b)
{
  this->dataPtr->blendMode = _b;
}

//////////////////////////////////////////////////
Material::BlendMode Material::Blend() const
{
  return this->dataPtr->blendMode;
}

//////////////////////////////////////////////////
void Material::SetShade(Material::ShadeMode _s)
{
  this->dataPtr->shadeMode = _s;
}

//////////////////////////////////////////////////
Material::ShadeMode Material::Shade() const
{
  return this->dataPtr->shadeMode;
}

//////////////////////////////////////////////////
void Material::SetPointSize(double _size)
{
  this->dataPtr->pointSize = _size;
}

//////////////////////////////////////////////////
double Material::PointSize() const
{
  return this->dataPtr->pointSize;
}
//////////////////////////////////////////////////
void Material::SetDepthWrite(bool _value)
{
  this->dataPtr->depthWrite = _value;
}

//////////////////////////////////////////////////
bool Material::DepthWrite() const
{
  return this->dataPtr->depthWrite;
}

//////////////////////////////////////////////////
void Material::SetLighting(bool _value)
{
  this->dataPtr->lighting = _value;
}

//////////////////////////////////////////////////
bool Material::Lighting() const
{
  return this->dataPtr->lighting;
}
