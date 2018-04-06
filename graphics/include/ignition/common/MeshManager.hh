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
#ifndef IGNITION_COMMON_MESHMANAGER_HH_
#define IGNITION_COMMON_MESHMANAGER_HH_

#include <map>
#include <utility>
#include <string>
#include <vector>
#include <memory>

#include <ignition/math/Plane.hh>
#include <ignition/math/Matrix3.hh>
#include <ignition/math/Matrix4.hh>
#include <ignition/math/Vector2.hh>
#include <ignition/math/Vector3.hh>
#include <ignition/math/Pose3.hh>

#include <ignition/common/graphics/Types.hh>
#include <ignition/common/SingletonT.hh>
#include <ignition/common/graphics/Export.hh>
#include <ignition/common/SuppressWarning.hh>

namespace ignition
{
  namespace common
  {
    /// \brief forward declaration
    class Mesh;
    class SubMesh;
    class MeshManagerPrivate;

    /// \class MeshManager MeshManager.hh ignition/common/MeshManager.hh
    /// \brief Maintains and manages all meshes
    class IGNITION_COMMON_GRAPHICS_VISIBLE MeshManager
        : public SingletonT<MeshManager>
    {
      /// \brief Constructor
      private: MeshManager();

      /// \brief Destructor.
      ///
      /// Destroys the collada loader, the stl loader and all the meshes
      private: virtual ~MeshManager();

      /// \brief Load a mesh from a file
      /// \param[in] _filename the path to the mesh
      /// \return a pointer to the created mesh
      public: const Mesh *Load(const std::string &_filename);

      /// \brief Export a mesh to a file
      /// \param[in] _mesh Pointer to the mesh to be exported
      /// \param[in] _filename Exported file's path and name
      /// \param[in] _extension Exported file's format ("dae" for Collada)
      /// \param[in] _exportTextures True to export texture images to
      /// '../materials/textures' folder
      public: void Export(const Mesh *_mesh, const std::string &_filename,
          const std::string &_extension, bool _exportTextures = false);

      /// \brief Checks a path extension against the list of valid extensions.
      /// \return true if the file extension is loadable
      public: bool IsValidFilename(const std::string &_filename);

      /// \brief Get mesh aabb and center.
      /// \param[in] _mesh the mesh
      /// \param[out] _center the AAB center position
      /// \param[out] _min_xyz the bounding box minimum
      /// \param[out] _max_xyz the bounding box maximum
      public: void MeshAABB(const Mesh *_mesh,
                  ignition::math::Vector3d &_center,
                  ignition::math::Vector3d &_min_xyz,
                  ignition::math::Vector3d &_max_xyz);

      /// \brief generate spherical texture coordinates
      /// \param[in] _mesh Pointer to the mesh
      /// \param[in] _center Center of the mesh
      public: void GenSphericalTexCoord(const Mesh *_mesh,
                  const ignition::math::Vector3d &_center);

      /// \brief Add a mesh to the manager.
      ///
      /// This MeshManager takes ownership of the mesh and will destroy it.
      /// See ~MeshManager.
      /// \param[in] the mesh to add.
      public: void AddMesh(Mesh *_mesh);

      /// \brief Get a mesh by name.
      /// \param[in] _name the name of the mesh to look for
      /// \return the mesh or nullptr if not found
      public: const ignition::common::Mesh *MeshByName(
                  const std::string &_name) const;

      /// \brief Return true if the mesh exists.
      /// \param[in] _name the name of the mesh
      public: bool HasMesh(const std::string &_name) const;

      /// \brief Create a sphere mesh.
      /// \param[in] _name the name of the mesh
      /// \param[in] _radius radius of the sphere in meter
      /// \param[in] _rings number of circles on th y axis
      /// \param[in] _segments number of segment per circle
      public: void CreateSphere(const std::string &_name, const float _radius,
                                const int _rings, const int _segments);

      /// \brief Create a Box mesh
      /// \param[in] _name the name of the new mesh
      /// \param[in] _sides the x y x dimentions of eah side in meter
      /// \param[in] _uvCoords the texture coordinates
      public: void CreateBox(const std::string &_name,
                             const ignition::math::Vector3d &_sides,
                             const ignition::math::Vector2d &_uvCoords);

      /// \brief Create an extruded mesh from polylines. The polylines are
      /// assumed to be closed and non-intersecting. Delaunay triangulation is
      /// applied to create the resulting mesh. If there is more than one
      /// polyline, a ray casting algorithm will be used to identify the
      /// exterior/interior edges and remove holes from the 2D shape before
      /// extrusion.
      /// \param[in] _name the name of the new mesh
      /// \param[in] _vertices A multidimensional vector of polylines and their
      /// vertices. Each element in the outer vector consists of a vector of
      /// vertices that describe one polyline.
      /// edges and remove the holes in the shape.
      /// \param[in] _height the height of extrusion
      public: void CreateExtrudedPolyline(const std::string &_name,
                  const std::vector<std::vector<ignition::math::Vector2d> >
                  &_vertices, const double _height);

      /// \brief Create a cylinder mesh
      /// \param[in] _name the name of the new mesh
      /// \param[in] _radius the radius of the cylinder in the x y plane
      /// \param[in] _height the height along z
      /// \param[in] _rings the number of circles along the height
      /// \param[in] _segments the number of segment per circle
      public: void CreateCylinder(const std::string &_name,
                                  const float _radius,
                                  const float _height,
                                  const int _rings,
                                  const int _segments);

      /// \brief Create a cone mesh
      /// \param[in] _name the name of the new mesh
      /// \param[in] _radius the radius of the cylinder in the x y plane
      /// \param[in] _height the height along z
      /// \param[in] _rings the number of circles along the height
      /// \param[in] _segments the number of segment per circle
      public: void CreateCone(const std::string &_name,
                              const float _radius,
                              const float _height,
                              const int _rings,
                              const int _segments);

      /// \brief Create a tube mesh.
      ///
      /// Generates rings inside and outside the cylinder
      /// Needs at least two rings and 3 segments
      /// \param[in] _name the name of the new mesh
      /// \param[in] _innerRadius the inner radius of the tube in the x y plane
      /// \param[in] _outterRadius the outer radius of the tube in the x y plane
      /// \param[in] _height the height along z
      /// \param[in] _rings the number of circles along the height
      /// \param[in] _segments the number of segment per circle
      /// \param[in] _arc the arc angle in radians
      public: void CreateTube(const std::string &_name,
                              const float _innerRadius,
                              const float _outterRadius,
                              const float _height,
                              const int _rings,
                              const int _segments,
                              const double _arc = 2.0 * IGN_PI);

      /// \brief Create mesh for a plane
      /// \param[in] _name
      /// \param[in] _plane plane parameters
      /// \param[in] _segments number of segments in x and y
      /// \param[in] _uvTile the texture tile size in x and y
      public: void CreatePlane(const std::string &_name,
                               const ignition::math::Planed &_plane,
                               const ignition::math::Vector2d &_segments,
                               const ignition::math::Vector2d &_uvTile);

      /// \brief Create mesh for a plane
      /// \param[in] _name the name of the new mesh
      /// \param[in] _normal the normal to the plane
      /// \param[in] _d distance from the origin along normal
      /// \param[in] _size the size of the plane in x and y
      /// \param[in] _segments the number of segments in x and y
      /// \param[in] _uvTile the texture tile size in x and y
      public: void CreatePlane(const std::string &_name,
                               const ignition::math::Vector3d &_normal,
                               const double _d,
                               const ignition::math::Vector2d &_size,
                               const ignition::math::Vector2d &_segments,
                               const ignition::math::Vector2d &_uvTile);

      /// \brief Tesselate a 2D mesh
      ///
      /// Makes a zigzag pattern compatible with strips
      /// \param[in] _sm the mesh to tesselate
      /// \param[in] _meshWith mesh width
      /// \param[in] _meshHeight the mesh height
      /// \param[in] _doubleSided flag to specify single or double sided
      private: void Tesselate2DMesh(SubMesh *_sm,
                                    const int _meshWidth,
                                    const int _meshHeight,
                                    const bool _doubleSided);

      /// \brief Create a Camera mesh
      /// \param[in] _name name of the new mesh
      /// \param[in] _scale scaling factor for the camera
      public: void CreateCamera(const std::string &_name, const float _scale);

      /// \brief Create a boolean mesh from two meshes
      /// \param[in] _name the name of the new mesh
      /// \param[in] _m1 the parent mesh in the boolean operation
      /// \param[in] _m2 the child mesh in the boolean operation
      /// \param[in] _operation the boolean operation applied to the two meshes
      /// \param[in] _offset _m2's pose offset from _m1
      public: void CreateBoolean(const std::string &_name, const Mesh *_m1,
          const Mesh *_m2, const int _operation,
          const ignition::math::Pose3d &_offset = ignition::math::Pose3d::Zero);

      /// \brief Converts a vector of polylines into a table of vertices and
      /// a list of edges (each made of 2 points from the table of vertices.
      /// \param[in] _polys the polylines
      /// \param[in] _tol tolerence for 2 vertices to be considered the same
      /// \param[out] _vertices a table of unique vertices
      /// \param[out] _edges a list of edges (made of start/end point indices
      /// from the vertex table)
      private: static void ConvertPolylinesToVerticesAndEdges(
                   const std::vector<std::vector<ignition::math::Vector2d> >
                   &_polys,
                   const double _tol,
                   std::vector<ignition::math::Vector2d> &_vertices,
                   std::vector<ignition::math::Vector2i> &_edges);

      /// \brief Check a point againts a list, and only adds it to the list
      /// if it is not there already.
      /// \param[in,out] _vertices the vertex table where points are stored
      /// \param[in] _p the point coordinates
      /// \param[in] _tol the maximum distance under which 2 points are
      /// considered to be the same point.
      /// \return the index of the point.
      private: static size_t AddUniquePointToVerticesTable(
                      std::vector<ignition::math::Vector2d> &_vertices,
                      const ignition::math::Vector2d &_p,
                      const double _tol);

      IGN_COMMON_WARN_IGNORE__DLL_INTERFACE_MISSING
      /// \brief Pointer to private data
      private: std::unique_ptr<MeshManagerPrivate> dataPtr;
      IGN_COMMON_WARN_RESUME__DLL_INTERFACE_MISSING

      /// \brief Singleton implementation
      private: friend class SingletonT<MeshManager>;
    };
  }
}
#endif
