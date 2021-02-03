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
#ifndef IGNITION_COMMON_COLLADAEXPORTER_HH_
#define IGNITION_COMMON_COLLADAEXPORTER_HH_

#include <memory>
#include <string>
#include <vector>

#include <ignition/common/MeshExporter.hh>
#include <ignition/common/graphics/Export.hh>

#include <ignition/math/Matrix4.hh>

#include <ignition/utils/ImplPtr.hh>

namespace ignition
{
  namespace common
  {
    /// \brief Class used to export Collada mesh files
    class IGNITION_COMMON_GRAPHICS_VISIBLE ColladaExporter : public MeshExporter
    {
      /// \brief Constructor
      public: ColladaExporter();

      /// \brief Destructor
      public: virtual ~ColladaExporter();

      /// \brief Export a mesh to a file
      /// \param[in] _mesh Pointer to the mesh to be exported
      /// \param[in] _filename Exported file's path and name
      /// \param[in] _exportTextures True to export texture images to
      /// '../materials/textures' folder
      public: virtual void Export(const Mesh *_mesh,
          const std::string &_filename, bool _exportTextures = false);

      public: void Export(const Mesh *_mesh,
          const std::string &_filename, bool _exportTextures,
          const std::vector<math::Matrix4d> &_submeshToMatrix);

      /// \brief Pointer to private data.
      IGN_UTILS_IMPL_PTR(dataPtr)
    };
  }
}
#endif
