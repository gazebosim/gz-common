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
#ifndef IGNITION_COMMON_MATERIALDENSITY_HH_
#define IGNITION_COMMON_MATERIALDENSITY_HH_

#include <string>
#include <limits>
#include <map>
#include <tuple>
#include "ignition/common/Export.hh"

namespace ignition
{
  namespace common
  {
    /// \brief Encapsulates density types.
    class IGNITION_COMMON_VISIBLE MaterialDensity
    {
      /// \enum Type
      /// \brief Types of materials.
      /// Source: https://en.wikipedia.org/wiki/Density
      public: enum class Type
      {
        /// \internal
        /// \brief Indicator used to create an iterator over the enum. Do not
        /// use this.
        BEGIN = 0,

        /// \brief Styrofoam, density = 75.0 kg/m^3
        STYROFOAM = BEGIN,

        /// \brief Pine, density = 373.0 kg/m^3
        PINE,

        /// \brief Wood, density = 700.0 kg/m^3
        WOOD,

        /// \brief Oak, density = 710.0 kg/m^3
        OAK,

        /// \brief Ice, density = 916.0 kg/m^3
        ICE,

        /// \brief Water, density = 1000.0 kg/m^3
        WATER,

        /// \brief Plastic, density = 1175.0 kg/m^3
        PLASTIC,

        /// \brief Concrete, density = 2000.0 kg/m^3
        CONCRETE,

        /// \brief Aluminum, density = 2700.0 kg/m^3
        ALUMINUM,

        /// \brief Steel alloy, density = 7600.0 kg/m^3
        STEEL_ALLOY,

        /// \brief Stainless steel, density = 7800.0 kg/m^3
        STEEL_STAINLESS,

        /// \brief Iron, density = 7870.0 kg/m^3
        IRON,

        /// \brief Brass, density = 8600.0 kg/m^3
        BRASS,

        /// \brief Copper, density = 8940.0 kg/m^3
        COPPER,

        /// \brief Tungsten, density = 19300.0 kg/m^3
        TUNGSTEN,

        /// \internal
        /// \brief Indicator used to create an iterator over the enum. Do not
        /// use this.
        END
      };

      /// \brief Accessor for retrieving density entries
      /// \return List of entries.
      public: static const std::map<Type, double> &Materials();

      /// \brief Return the density of the given material name, or -1
      /// if the material is not found.
      /// \param[in] _material Name of the material, See Type.
      /// \return Matching density if found, otherwise -1.
      public: static double Density(const std::string &_material);

      /// \brief Return the density of a material.
      /// \param[in] _material Type of the material, See Type.
      /// \return Matching density if found, otherwise -1.
      public: static double Density(const Type _material);

      /// \brief Return the material with the closest density value within
      /// _epsilon, or MATERIAL_TYPE_END if not found.
      /// \param[in] _value Density value of entry to match.
      /// \param[in] _epsilon Allowable range of difference between _value,
      /// and a material's density.
      /// \return A tuple where the first element is the Type, and
      /// the second the density. A value of {MATERIAL_TYPE_END, -1} is
      /// returned on error.
      public: static std::tuple<Type, double> Nearest(
                  const double _value,
                  const double _epsilon = std::numeric_limits<double>::max());

      /// \brief Return the material with the closest density value within
      /// _epsilon, or MATERIAL_TYPE_END if not found.
      /// \param[in] _value Density value of entry to match.
      /// \param[in] _epsilon Allowable range of difference between _value,
      /// and a material's density.
      /// \return The nearest material type. MATERIAL_TYPE_END on error.
      public: static Type NearestMaterial(const double _value,
                  const double _epsilon = std::numeric_limits<double>::max());

#ifdef _WIN32
// Disable warning C4251
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
      /// \brief List of density entries
      private: static std::map<Type, double> materials;
#ifdef _WIN32
#pragma warning(pop)
#endif
    };
  }
}
#endif
