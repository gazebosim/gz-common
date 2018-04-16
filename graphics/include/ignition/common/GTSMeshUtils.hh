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

#ifndef IGNITION_COMMON_GTSMESHUTILS_HH_
#define IGNITION_COMMON_GTSMESHUTILS_HH_

#include <vector>

#include <ignition/math/Vector2.hh>
#include <ignition/common/graphics/Export.hh>


struct _GtsSurface;
typedef _GtsSurface GtsSurface;

namespace ignition
{
  namespace common
  {
    class SubMesh;

    /// \class GTSMeshUtils GTSMeshUtils.hh ignition/common/GTSMeshUtils.hh
    /// \brief Creates GTS utilities for meshes
    class IGNITION_COMMON_GRAPHICS_VISIBLE GTSMeshUtils
    {
      /// \brief Perform delaunay triangulation on input vertices.
      /// \param[in] _vertices A list of all vertices
      /// \param[in] _edges A list of edges. Each edge is made of 2 vertex
      /// indices from _vertices
      /// \param[out] _submesh A submesh that will be populated with the
      /// resulting triangles.
      /// \return True on success.
      public: static bool DelaunayTriangulation(
                  const std::vector<ignition::math::Vector2d> &_vertices,
                  const std::vector<ignition::math::Vector2i> &_edges,
                  SubMesh *_submesh);

      /// \brief Perform delaunay triangulation on input vertices.
      /// \param[in] _vertices A list of all vertices
      /// \param[in] _edges A list of edges. Each edge is made of 2 vertex
      /// indices from _vertices
      /// \return Triangulated GTS surface.
      private: static GtsSurface *DelaunayTriangulation(
                   const std::vector<ignition::math::Vector2d> &_vertices,
                   const std::vector<ignition::math::Vector2i> &_edges);
    };
  }
}
#endif
