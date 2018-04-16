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
#include <ignition/common/NodeTransform.hh>

using namespace ignition;
using namespace common;

/// \brief Private data for NodeTransform
class ignition::common::NodeTransformPrivate
{
  /// \brief the sid
  public: std::string sid;

  /// \brief transform type
  public: NodeTransformType type;

  /// \brief transform
  public: math::Matrix4d transform;

  /// \brief source data values (can be a matrix, a position or rotation)
  public: std::vector<double> source;
};

//////////////////////////////////////////////////
NodeTransform::NodeTransform(const NodeTransformType _type)
  : data(new NodeTransformPrivate)
{
  this->data->sid = "_default_";
  this->data->type = _type;
  this->data->transform = math::Matrix4d::Identity;
}

//////////////////////////////////////////////////
NodeTransform::NodeTransform(const math::Matrix4d &_mat,
    const std::string &_sid, const NodeTransformType _type)
  : data(new NodeTransformPrivate)
{
  this->data->sid = _sid;
  this->data->type = _type;
  this->data->transform = _mat;
}

//////////////////////////////////////////////////
NodeTransform::~NodeTransform()
{
  delete this->data;
  this->data = NULL;
}

//////////////////////////////////////////////////
void NodeTransform::Set(const math::Matrix4d &_mat)
{
  this->data->transform = _mat;
}

//////////////////////////////////////////////////
void NodeTransform::SetType(const NodeTransformType _type)
{
  this->data->type = _type;
}

//////////////////////////////////////////////////
void NodeTransform::SetSID(const std::string &_sid)
{
  this->data->sid = _sid;
}

//////////////////////////////////////////////////
math::Matrix4d NodeTransform::Get() const
{
  return this->data->transform;
}

//////////////////////////////////////////////////
NodeTransformType NodeTransform::Type() const
{
  return this->data->type;
}

//////////////////////////////////////////////////
std::string NodeTransform::SID() const
{
  return this->data->sid;
}

//////////////////////////////////////////////////
void NodeTransform::SetComponent(const unsigned int _idx, const double _value)
{
  this->data->source[_idx] = _value;
}

//////////////////////////////////////////////////
void NodeTransform::SetSourceValues(const math::Matrix4d &_mat)
{
  this->data->source.resize(16);
  unsigned int idx = 0;
  for (unsigned int i = 0; i < 4; ++i)
  {
    for (unsigned int j = 0; j < 4; ++j, ++idx)
    {
      this->data->source[idx] = _mat(i, j);
    }
  }
}

//////////////////////////////////////////////////
void NodeTransform::SetSourceValues(const math::Vector3d &_vec)
{
  this->data->source.resize(3);
  this->data->source[0] = _vec.X();
  this->data->source[1] = _vec.Y();
  this->data->source[2] = _vec.Z();
}

//////////////////////////////////////////////////
void NodeTransform::SetSourceValues(const math::Vector3d &_axis,
    const double _angle)
{
  this->data->source.resize(4);
  this->data->source[0] = _axis.X();
  this->data->source[1] = _axis.Y();
  this->data->source[2] = _axis.Z();
  this->data->source[3] = _angle;
}

//////////////////////////////////////////////////
void NodeTransform::RecalculateMatrix()
{
  if (this->data->type == MATRIX)
  {
    this->data->transform.Set(
        this->data->source[0], this->data->source[1], this->data->source[2],
        this->data->source[3], this->data->source[4], this->data->source[5],
        this->data->source[6], this->data->source[7], this->data->source[8],
        this->data->source[9], this->data->source[10], this->data->source[11],
        this->data->source[12], this->data->source[13], this->data->source[14],
        this->data->source[15]);
  }
  else
  {
    if (this->data->type == TRANSLATE)
    {
      this->data->transform.SetTranslation(math::Vector3d(this->data->source[0],
            this->data->source[1], this->data->source[2]));
    }
    else
    {
      if (this->data->type == ROTATE)
      {
        math::Matrix3d mat;
        mat.Axis(math::Vector3d(this->data->source[0],
                                this->data->source[1],
                                this->data->source[2]),
                 IGN_DTOR(this->data->source[3]));
        this->data->transform = mat;
      }
      else
      {
        this->data->transform.Scale(math::Vector3d(this->data->source[0],
            this->data->source[1], this->data->source[2]));
      }
    }
  }
}

//////////////////////////////////////////////////
math::Matrix4d NodeTransform::operator()() const
{
  return this->data->transform;
}

//////////////////////////////////////////////////
math::Matrix4d NodeTransform::operator*(const NodeTransform &_t) const
{
  return this->data->transform * _t();
}

//////////////////////////////////////////////////
math::Matrix4d NodeTransform::operator*(const math::Matrix4d &_m) const
{
  return this->data->transform * _m;
}

//////////////////////////////////////////////////
void NodeTransform::PrintSource() const
{
  std::cout << this->data->sid;
  for (unsigned int i = 0; i < this->data->source.size(); ++i)
    std::cout << " " << this->data->source[i];
  std::cout << "\n";
}
