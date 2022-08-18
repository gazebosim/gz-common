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
#ifndef IGNITION_COMMON_MESHEXPORTER_HH_
#define IGNITION_COMMON_MESHEXPORTER_HH_

#include <string>
#include <ignition/common/graphics/Export.hh>

namespace ignition
{
  namespace common
  {
    class Mesh;

    /// \brief Base class for exporting meshes
    class IGNITION_COMMON_GRAPHICS_VISIBLE MeshExporter
    {
      /// \brief Constructor
      public: MeshExporter();

      /// \brief Destructor
      public: virtual ~MeshExporter();

      /// \brief Export a mesh to a file
      /// \param[in] _mesh Pointer to the mesh to be exported
      /// \param[in] _filename Exported file's path and name
      /// \param[in] _exportTextures True to export texture images to
      /// '../materials/textures' folder
      public: virtual void Export(const Mesh *_mesh,
          const std::string &_filename, bool _exportTextures = false) = 0;
    };
  }
}
#endif
