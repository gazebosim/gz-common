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
#include <string>
#include <algorithm>

#include "ignition/math/Helpers.hh"

#include "ignition/common/Console.hh"
#include "ignition/common/Material.hh"
#include "ignition/common/SubMesh.hh"

using namespace ignition;
using namespace common;

/// \brief Private data for SubMesh
class ignition::common::SubMeshPrivate
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
  public: SubMesh::PrimitiveType primitiveType = SubMesh::TRIANGLES;

  /// \brief The material index for this mesh. Relates to the parent
  /// mesh material list.
  public: int materialIndex = -1;

  /// \brief The name of the sub-mesh
  public: std::string name;
};

//////////////////////////////////////////////////
SubMesh::SubMesh()
: dataPtr(new SubMeshPrivate)
{
}

//////////////////////////////////////////////////
SubMesh::SubMesh(const std::string &_name)
: dataPtr(new SubMeshPrivate)
{
  this->dataPtr->name = _name;
}

//////////////////////////////////////////////////
SubMesh::SubMesh(const SubMesh &_submesh)
: dataPtr(new SubMeshPrivate)
{
  this->dataPtr->name = _submesh.dataPtr->name;
  this->dataPtr->materialIndex = _submesh.dataPtr->materialIndex;
  this->dataPtr->primitiveType = _submesh.dataPtr->primitiveType;

  std::copy(_submesh.dataPtr->nodeAssignments.begin(),
      _submesh.dataPtr->nodeAssignments.end(),
      std::back_inserter(this->dataPtr->nodeAssignments));

  std::copy(_submesh.dataPtr->indices.begin(),
      _submesh.dataPtr->indices.end(),
      std::back_inserter(this->dataPtr->indices));
  std::copy(_submesh.dataPtr->normals.begin(),
      _submesh.dataPtr->normals.end(),
      std::back_inserter(this->dataPtr->normals));
  std::copy(_submesh.dataPtr->texCoords.begin(),
      _submesh.dataPtr->texCoords.end(),
      std::back_inserter(this->dataPtr->texCoords));
  std::copy(_submesh.dataPtr->vertices.begin(),
      _submesh.dataPtr->vertices.end(),
      std::back_inserter(this->dataPtr->vertices));
}

//////////////////////////////////////////////////
SubMesh::~SubMesh()
{
  this->dataPtr->vertices.clear();
  this->dataPtr->indices.clear();
  this->dataPtr->nodeAssignments.clear();
}

//////////////////////////////////////////////////
void SubMesh::SetPrimitiveType(PrimitiveType _type)
{
  this->dataPtr->primitiveType = _type;
}

//////////////////////////////////////////////////
SubMesh::PrimitiveType SubMesh::SubMeshPrimitiveType() const
{
  return this->dataPtr->primitiveType;
}

//////////////////////////////////////////////////
void SubMesh::AddIndex(const unsigned int _index)
{
  this->dataPtr->indices.push_back(_index);
}

//////////////////////////////////////////////////
void SubMesh::AddVertex(const ignition::math::Vector3d &_v)
{
  this->dataPtr->vertices.push_back(_v);
}

//////////////////////////////////////////////////
void SubMesh::AddVertex(const double _x, const double _y, const double _z)
{
  this->AddVertex(ignition::math::Vector3d(_x, _y, _z));
}

//////////////////////////////////////////////////
void SubMesh::AddNormal(const ignition::math::Vector3d &_n)
{
  this->dataPtr->normals.push_back(_n);
}

//////////////////////////////////////////////////
void SubMesh::AddNormal(const double _x, const double _y, const double _z)
{
  this->AddNormal(ignition::math::Vector3d(_x, _y, _z));
}

//////////////////////////////////////////////////
void SubMesh::AddTexCoord(const double _u, const double _v)
{
  this->dataPtr->texCoords.push_back(ignition::math::Vector2d(_u, _v));
}

//////////////////////////////////////////////////
void SubMesh::AddTexCoord(const ignition::math::Vector2d &_uv)
{
  this->dataPtr->texCoords.push_back(_uv);
}

//////////////////////////////////////////////////
void SubMesh::AddNodeAssignment(const unsigned int _vertex,
    const unsigned int _node, const float _weight)
{
  NodeAssignment na;
  na.vertexIndex = _vertex;
  na.nodeIndex = _node;
  na.weight = _weight;

  this->dataPtr->nodeAssignments.push_back(na);
}

//////////////////////////////////////////////////
ignition::math::Vector3d SubMesh::Vertex(const unsigned int _index) const
{
  if (_index >= this->dataPtr->vertices.size())
  {
    ignerr << "Index too large" << std::endl;
    return math::Vector3d::Zero;
  }

  return this->dataPtr->vertices[_index];
}

//////////////////////////////////////////////////
bool SubMesh::HasVertex(const unsigned int _index) const
{
  return _index < this->dataPtr->vertices.size();
}

//////////////////////////////////////////////////
void SubMesh::SetVertex(const unsigned int _index,
    const ignition::math::Vector3d &_v)
{
  if (_index >= this->dataPtr->vertices.size())
  {
    ignerr << "Index too large" << std::endl;
    return;
  }

  this->dataPtr->vertices[_index] = _v;
}

//////////////////////////////////////////////////
ignition::math::Vector3d SubMesh::Normal(const unsigned int _index) const
{
  if (_index >= this->dataPtr->normals.size())
  {
    ignerr << "Index too large" << std::endl;
    return math::Vector3d::Zero;
  }

  return this->dataPtr->normals[_index];
}

//////////////////////////////////////////////////
bool SubMesh::HasNormal(const unsigned int _index) const
{
  return _index < this->dataPtr->normals.size();
}

//////////////////////////////////////////////////
bool SubMesh::HasTexCoord(const unsigned int _index) const
{
  return _index < this->dataPtr->texCoords.size();
}

//////////////////////////////////////////////////
bool SubMesh::HasNodeAssignment(const unsigned int _index) const
{
  return _index < this->dataPtr->nodeAssignments.size();
}

//////////////////////////////////////////////////
void SubMesh::SetNormal(const unsigned int _index,
    const ignition::math::Vector3d &_n)
{
  if (_index >= this->dataPtr->normals.size())
  {
    ignerr << "Index too large" << std::endl;
    return;
  }

  this->dataPtr->normals[_index] = _n;
}

//////////////////////////////////////////////////
ignition::math::Vector2d SubMesh::TexCoord(const unsigned int _index) const
{
  if (_index >= this->dataPtr->texCoords.size())
  {
    ignerr << "Index too large" << std::endl;
    return math::Vector2d::Zero;
  }

  return this->dataPtr->texCoords[_index];
}

//////////////////////////////////////////////////
void SubMesh::SetTexCoord(const unsigned int _index,
    const ignition::math::Vector2d &_t)
{
  if (_index >= this->dataPtr->texCoords.size())
  {
    ignerr << "Index too large" << std::endl;
    return;
  }

  this->dataPtr->texCoords[_index] = _t;
}

//////////////////////////////////////////////////
int SubMesh::Index(const unsigned int _index) const
{
  if (_index >= this->dataPtr->indices.size())
  {
    ignerr << "Index too large" << std::endl;
    return -1;
  }

  return this->dataPtr->indices[_index];
}

//////////////////////////////////////////////////
void SubMesh::SetIndex(const unsigned int _index, const unsigned int _i)
{
  if (_index >= this->dataPtr->indices.size())
  {
    ignerr << "Index too large" << std::endl;
    return;
  }

  this->dataPtr->indices[_index] = _i;
}

//////////////////////////////////////////////////
NodeAssignment SubMesh::NodeAssignmentByIndex(
    const unsigned int _index) const
{
  if (_index >= this->dataPtr->nodeAssignments.size())
  {
    ignerr << "Index too large" << std::endl;
    return NodeAssignment();
  }

  return this->dataPtr->nodeAssignments[_index];
}

//////////////////////////////////////////////////
ignition::math::Vector3d SubMesh::Max() const
{
  if (this->dataPtr->vertices.empty())
    return ignition::math::Vector3d::Zero;

  ignition::math::Vector3d max;

  max.X(-ignition::math::MAX_F);
  max.Y(-ignition::math::MAX_F);
  max.Z(-ignition::math::MAX_F);

  for (const auto &v : this->dataPtr->vertices)
  {
    max.X(std::max(max.X(), v.X()));
    max.Y(std::max(max.Y(), v.Y()));
    max.Z(std::max(max.Z(), v.Z()));
  }

  return max;
}

//////////////////////////////////////////////////
ignition::math::Vector3d SubMesh::Min() const
{
  if (this->dataPtr->vertices.empty())
    return ignition::math::Vector3d::Zero;

  ignition::math::Vector3d min;

  min.X(ignition::math::MAX_F);
  min.Y(ignition::math::MAX_F);
  min.Z(ignition::math::MAX_F);

  for (const auto &v : this->dataPtr->vertices)
  {
    min.X(std::min(min.X(), v.X()));
    min.Y(std::min(min.Y(), v.Y()));
    min.Z(std::min(min.Z(), v.Z()));
  }

  return min;
}

//////////////////////////////////////////////////
unsigned int SubMesh::VertexCount() const
{
  return this->dataPtr->vertices.size();
}

//////////////////////////////////////////////////
unsigned int SubMesh::NormalCount() const
{
  return this->dataPtr->normals.size();
}

//////////////////////////////////////////////////
unsigned int SubMesh::IndexCount() const
{
  return this->dataPtr->indices.size();
}

//////////////////////////////////////////////////
unsigned int SubMesh::TexCoordCount() const
{
  return this->dataPtr->texCoords.size();
}

//////////////////////////////////////////////////
unsigned int SubMesh::NodeAssignmentsCount() const
{
  return this->dataPtr->nodeAssignments.size();
}

//////////////////////////////////////////////////
unsigned int SubMesh::MaxIndex() const
{
  auto maxIter = std::max_element(this->dataPtr->indices.begin(),
      this->dataPtr->indices.end());

  if (maxIter != this->dataPtr->indices.end())
    return *maxIter;

  return 0;
}

//////////////////////////////////////////////////
void SubMesh::SetMaterialIndex(const unsigned int _index)
{
  this->dataPtr->materialIndex = _index;
}

//////////////////////////////////////////////////
unsigned int SubMesh::MaterialIndex() const
{
  return this->dataPtr->materialIndex;
}

//////////////////////////////////////////////////
bool SubMesh::HasVertex(const ignition::math::Vector3d &_v) const
{
  for (const auto &v : this->dataPtr->vertices)
    if (_v.Equal(v))
      return true;

  return false;
}

//////////////////////////////////////////////////
int SubMesh::IndexOfVertex(const ignition::math::Vector3d &_v) const
{
  for (auto iter = this->dataPtr->vertices.begin();
      iter != this->dataPtr->vertices.end(); ++iter)
  {
    if (_v.Equal(*iter))
      return iter - this->dataPtr->vertices.begin();
  }
  return -1;
}

//////////////////////////////////////////////////
void SubMesh::FillArrays(double **_vertArr, int **_indArr) const
{
  if (this->dataPtr->vertices.empty() || this->dataPtr->indices.empty())
  {
    ignerr << "No vertices or indices\n";
    return;
  }

  if (*_vertArr)
    delete [] *_vertArr;

  if (*_indArr)
    delete [] *_indArr;

  *_vertArr = new double[this->dataPtr->vertices.size() * 3];
  *_indArr = new int[this->dataPtr->indices.size()];

  unsigned int vi = 0;
  for (auto &v : this->dataPtr->vertices)
  {
    (*_vertArr)[vi++] = static_cast<float>(v.X());
    (*_vertArr)[vi++] = static_cast<float>(v.Y());
    (*_vertArr)[vi++] = static_cast<float>(v.Z());
  }

  unsigned int ii = 0;
  for (auto &i : this->dataPtr->indices)
  {
    (*_indArr)[ii++] = i;
  }
}

//////////////////////////////////////////////////
void SubMesh::RecalculateNormals()
{
  if (this->dataPtr->normals.size() < 3u)
    return;

  // Reset all the normals
  for (auto &n : this->dataPtr->normals)
    n.Set(0, 0, 0);

  if (this->dataPtr->normals.size() != this->dataPtr->vertices.size())
    this->dataPtr->normals.resize(this->dataPtr->vertices.size());

  // For each face, which is defined by three indices, calculate the normals
  for (unsigned int i = 0; i < this->dataPtr->indices.size(); i+= 3)
  {
    ignition::math::Vector3d v1 =
        this->dataPtr->vertices[this->dataPtr->indices[i]];
    ignition::math::Vector3d v2 =
        this->dataPtr->vertices[this->dataPtr->indices[i+1]];
    ignition::math::Vector3d v3 =
        this->dataPtr->vertices[this->dataPtr->indices[i+2]];
    ignition::math::Vector3d n = ignition::math::Vector3d::Normal(v1, v2, v3);

    for (unsigned int j = 0; j < this->dataPtr->vertices.size(); ++j)
    {
      ignition::math::Vector3d v = this->dataPtr->vertices[j];
      if (v == v1 || v == v2 || v == v3)
      {
        this->dataPtr->normals[j] += n;
      }
    }
  }

  // Normalize the results
  for (auto &n : this->dataPtr->normals)
  {
    n.Normalize();
  }
}

//////////////////////////////////////////////////
void SubMesh::GenSphericalTexCoord(const ignition::math::Vector3d &_center)
{
  this->dataPtr->texCoords.clear();

  for (const auto &vert : this->dataPtr->vertices)
  {
    // generate projected texture coordinates, projected from center
    //  x, y, z for computing texture coordinate projections
    double x = vert.X() - _center.X();
    double y = vert.Y() - _center.Y();
    double z = vert.Z() - _center.Z();

    double r = std::max(0.000001, sqrt(x*x+y*y+z*z));
    double s = std::min(1.0, std::max(-1.0, z/r));
    double t = std::min(1.0, std::max(-1.0, y/r));
    double u = acos(s) / IGN_PI;
    double v = acos(t) / IGN_PI;
    this->AddTexCoord(u, v);
  }
}

//////////////////////////////////////////////////
void SubMesh::Scale(const ignition::math::Vector3d &_factor)
{
  for (auto &v : this->dataPtr->vertices)
    v *= _factor;
}

//////////////////////////////////////////////////
void SubMesh::Scale(const double &_factor)
{
  for (auto &v : this->dataPtr->vertices)
    v *= _factor;
}

//////////////////////////////////////////////////
void SubMesh::Center(const ignition::math::Vector3d &_center)
{
  ignition::math::Vector3d min, max, half;
  min = this->Min();
  max = this->Max();
  half = (max - min) * 0.5;

  this->Translate(_center - (min + half));
}

//////////////////////////////////////////////////
void SubMesh::Translate(const ignition::math::Vector3d &_vec)
{
  for (auto &v : this->dataPtr->vertices)
    v += _vec;
}

//////////////////////////////////////////////////
void SubMesh::SetName(const std::string &_name)
{
  this->dataPtr->name = _name;
}

//////////////////////////////////////////////////
std::string SubMesh::Name() const
{
  return this->dataPtr->name;
}

//////////////////////////////////////////////////
NodeAssignment::NodeAssignment()
  : vertexIndex(0), nodeIndex(0), weight(0.0)
{
}

