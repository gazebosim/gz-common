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

#ifndef _IGNITION_COMMON_MATERIAL_PRIVATE_HH_
#define _IGNITION_COMMON_MATERIAL_PRIVATE_HH_

#include "ignition/common/Material.hh"
#include "ignition/common/Color.hh"

namespace ignition
{
  namespace common
  {
    /// \internal
    /// \brief Private data for Material
    class MaterialPrivate
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
      public: Material::MaterialBlendMode blendMode;

      /// \brief the shade mode
      public: Material::MaterialShadeMode shadeMode;

      /// \brief the total number of instantiated Material instances
      public: static unsigned int counter;

      /// \brief flag to perform depth buffer write
      public: bool depthWrite;

      /// \brief flag to inidcate if lighting is enabled for this material.
      public: bool lighting;

      /// \brief source blend factor
      public: double srcBlendFactor;

      /// \brief destination blend factor
      public: double dstBlendFactor;
    };
  }
}
#endif
