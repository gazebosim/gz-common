/*
 * Copyright (C) 2015 Open Source Robotics Foundation
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

#ifndef _IGNITION_COMMON_MATERIAL_HH_
#define _IGNITION_COMMON_MATERIAL_HH_

#include <string>

#include "ignition/common/Color.hh"

namespace ignition
{
  namespace common
  {
    class MaterialPrivate;

    /// \class Material Material.hh ignition/common/Material.hh
    /// \brief Encapsulates description of a material
    class IGNITION_VISIBLE Material
    {
      /// \brief Enum used to indicate the type of material shading, i.e.
      /// what interpolation techinique to be applied to the colors of the
      /// surface
      public: enum MaterialShadeMode
      {
        /// \brief Flat shading. No interpolation.
        FLAT,

        /// \brief Gouraud shading. Interpolation of vertex colors across
        /// polygons.
        GOURAUD,

        /// \brief Phong shading. Interpolation of vertex normals across
        /// polygons, and final color is obtained using the Phong reflection
        /// model.
        PHONG,

        /// \brief Blinn-Phong shading. Gouraud shading with a modified Phong
        /// reflection model (computationally more efficient).
        BLINN,

        /// \brief Number of entries in this enum
        SHADE_COUNT
      };

      /// \brief An array of strings for the shade mode enum.
      public: static std::string ShadeModeStr[SHADE_COUNT];

      /// \brief Enum used to indicate the type of blend mode for a material
      /// pass
      public: enum MaterialBlendMode
      {
        /// \brief Color is added to the scene
        ADD,

        /// \brief Color is multiplied with the scene contents
        MODULATE,

        /// \brief Color replaces the scene contents
        REPLACE,

        /// \brief Number of entries in this enum
        BLEND_COUNT
      };

      /// \brief An array of strings for the blend mode enum.
      public: static std::string BlendModeStr[BLEND_COUNT];

      /// \brief Constructor. Create material with a default white color.
      public: Material();

      /// \brief Destructor
      public: virtual ~Material();

      /// \brief Create a material with specified color
      /// \param[in] _clr Color of the material
      public: Material(const Color &_clr);

      /// \brief Get the name of the material
      /// \return The name of the material
      public: std::string Name() const;

      /// \brief Set a texture image
      /// \param[in] _tex The name of the texture, which must be in the
      ///             resource path
      public: void SetTextureImage(const std::string &_tex);

      /// \brief Set a texture image
      /// \param[in] _tex The name of the texture
      /// \param[in] _resourcePath Path which contains _tex
      public: void SetTextureImage(const std::string &_tex,
                                   const std::string &_resourcePath);

      /// \brief Get a texture image
      /// \return The name of the texture image (if one exists) or an empty
      /// string
      public: std::string TextureImage() const;

      /// \brief Set the ambient color
      /// \param[in] _clr The ambient color
      public: void SetAmbient(const Color &_clr);

      /// \brief Get the ambient color
      /// \return The ambient color
      public: Color Ambient() const;

      /// \brief Set the diffuse color
      /// \param[in] _clr The diffuse color
      public: void SetDiffuse(const Color &_clr);

      /// \brief Get the diffuse color
      /// \return The diffuse color
      public: Color Diffuse() const;

      /// \brief Set the specular color
      /// \param[in] _clr The specular color
      public: void SetSpecular(const Color &_clr);

      /// \brief Get the specular color
      /// \return The specular color
      public: Color Specular() const;

      /// \brief Set the emissive color
      /// \param[in] _clr The emissive color
      public: void SetEmissive(const Color &_clr);

      /// \brief Get the emissive color
      /// \return The emissive color
      public: Color Emissive() const;

      /// \brief Set the transparency percentage (0..1)
      /// \param[in] _t The amount of transparency (0..1)
      public: void SetTransparency(double _t);

      /// \brief Get the transparency percentage (0..1)
      /// \return The transparency percentage
      public: double Transparency() const;

      /// \brief Set the shininess
      /// \param[in] _t The shininess value
      public: void SetShininess(double _t);

      /// \brief Get the shininess
      /// \return The shininess value
      public: double Shininess() const;

      /// \brief Set the blend factors. Will be interpreted as:
      ///        (texture * _srcFactor) + (scene_pixel * _dstFactor)
      /// \param[in] _srcFactor The source factor
      /// \param[in] _dstFactor The destination factor
      public: void SetBlendFactors(double _srcFactor, double _dstFactor);

      /// \brief Get the blend factors
      /// \param[out] _srcFactor Source factor is returned in this variable
      /// \param[out] _dstFactor Destination factor is returned in this variable
      public: void BlendFactors(double &_srcFactor, double &_dstFactor) const;

      /// \brief Set the blending mode
      /// \param[in] _b the blend mode
      public: void SetBlendMode(MaterialBlendMode _b);

      /// \brief Get the blending mode
      /// \return the blend mode
      public: MaterialBlendMode BlendMode() const;

      /// \brief Set the shading mode
      /// param[in] the shading mode
      public: void SetShadeMode(MaterialShadeMode _b);

      /// \brief Get the shading mode
      /// \return the shading mode
      public: MaterialShadeMode ShadeMode() const;

      /// \brief Set the point size
      /// \param[in] _size the size
      public: void SetPointSize(double _size);

      /// \brief Get the point size
      /// \return the point size
      public: double PointSize() const;

      /// \brief Set depth write
      /// \param[in] _value the depth write enabled state
      public: void SetDepthWrite(bool _value);

      /// \brief Get depth write
      /// \return the depth write enabled state
      public: bool DepthWrite() const;

      /// \brief Set whether to enable dynamic lighting. Note that setting
      /// other material properties such as ambient/diffuse/specular,
      /// transparency, and shininess will also enable dynamic lighting.
      /// \param[in] _value the dynamic lighting enabled state
      public: void SetLighting(bool _value);

      /// \brief Get whether dynamic lighting is enabled
      /// \return the dynamic lighting enabled state
      public: bool Lighting() const;

      /// \brief Stream insertion operator
      /// param[in] _out the output stream to extract from
      /// param[out] _m the material information
      public: friend std::ostream &operator<<(std::ostream &_out,
                  const Material &_m);

      /// \brief Private data pointer
      private: MaterialPrivate *dataPtr;
    };
  }
}
#endif
