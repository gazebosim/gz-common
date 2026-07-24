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
#ifndef GZ_COMMON_COLLADALOADER_HH_
#define GZ_COMMON_COLLADALOADER_HH_

#include <string>

#include <gz/common/graphics/Export.hh>
#include <gz/common/MeshLoader.hh>

#include <gz/utils/ImplPtr.hh>

namespace gz
{
  namespace common
  {
    /// \class ColladaLoader ColladaLoader.hh gz/common/ColladaLoader.hh
    /// \brief Class used to load Collada mesh files
    class GZ_COMMON_GRAPHICS_VISIBLE ColladaLoader : public MeshLoader
    {
      /// \brief Constructor
      public: ColladaLoader();

      /// \brief Destructor
      public: virtual ~ColladaLoader();

      /// \brief Load a mesh
      /// \param[in] _filename Collada file to load
      /// \return Pointer to a new Mesh, owned by the caller. A null
      /// pointer is returned when the file does not exist, cannot be
      /// parsed, or its root element is not COLLADA. Other problems are
      /// reported through the console log and do not abort loading:
      /// a COLLADA version other than 1.4.0 or 1.4.1 is reported but the
      /// mesh is still loaded, and a scene that references a missing visual
      /// scene is reported and yields a valid mesh with no geometry.
      /// In general, malformed subelements are skipped and the loader
      /// returns whatever geometry could be recovered.
      public: virtual Mesh *Load(const std::string &_filename);

      /// \internal
      /// \brief Pointer to private data.
      GZ_UTILS_IMPL_PTR(dataPtr)
    };
  }
}
#endif
