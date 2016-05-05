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
#ifndef IGNITION_SUBMESH_PRIVATE_HH_
#define IGNITION_SUBMESH_PRIVATE_HH_

#include <vector>
#include <string>

#include <ignition/math/Vector3.hh>
#include <ignition/math/Vector2.hh>

#include <ignition/common/SubMesh.hh>

namespace ignition
{
  namespace common
  {
    class NodeAssignment;

    /// \internal
    /// \brief Private data for SubMesh
    class SubMeshPrivate
    {
      /// \brief the vertex array
      public: std::vector<ignition::math::Vector3d> vertices;

      /// \brief the normal array
      public: std::vector<ignition::math::Vector3d> normals;

      /// \brief the texture coordinate array
      public: std::vector<ignition::math::Vector2d> texCoords;

      /// \brief the vertex index array
      public: std::vector<unsigned int> indices;

      /// \brief node assignment array
      public: std::vector<NodeAssignment> nodeAssignments;

      /// \brief primitive type for the mesh
      public: SubMesh::PrimitiveType primitiveType;

      /// \brief The material index for this mesh. Relates to the parent
      /// mesh material list.
      public: int materialIndex;

      /// \brief The name of the sub-mesh
      public: std::string name;
    };
  }
}

#endif
