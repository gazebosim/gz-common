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
#ifndef IGNITION_COMMON_ASSIMPLOADER_HH_
#define IGNITION_COMMON_ASSIMPLOADER_HH_

#include <stdint.h>
#include <string>

#include <ignition/utils/ImplPtr.hh>

#include "ignition/common/MeshLoader.hh"
#include "ignition/common/graphics/Export.hh"

#include <assimp/Importer.hpp> // C++ importer interface
#include <assimp/scene.h> // Output data structure
#include <assimp/postprocess.h> // Post processing flags

namespace ignition
{
  namespace common
  {
    /// \class AssimpLoader AssimpLoader.hh ignition/common/AssimpLoader.hh
    /// \brief Class used to load FBX/GLTF2 mesh files
    class IGNITION_COMMON_GRAPHICS_VISIBLE AssimpLoader : public MeshLoader
    {
      /// \brief Constructor
      public: AssimpLoader();

      /// \brief Destructor
      public: virtual ~AssimpLoader();

      /// \brief Creates a new mesh and loads the data from a file
      /// \param[in] _filename the mesh file
      public: virtual Mesh *Load(const std::string &_filename);

      /// \brief Process assimp node
      /// \param[in] _node assimp node obj
      /// \param[in] _scene assimp scene obj
      /// \param[in] _mesh Gazebo mesh obj
      /// \param[out]
      /// \return true if read was successful
      private: bool ProcessNode(aiNode *_node, const aiScene *_scene, Mesh *_mesh);


      /// \brief Converts an assimp mesh into Gazebo mesh.
      /// \param[in] _node assimp node
      /// \param[out]  the mesh where to load the data
      /// \return true if read was successful
      private: SubMesh Assimp2GazeboMesh(aiMesh *_mesh);



      /// \brief Private data pointer.
      IGN_UTILS_IMPL_PTR(dataPtr)
    };
  }
}
#endif

