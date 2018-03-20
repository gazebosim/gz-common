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
#include <list>
#include <ignition/common/SkeletonAnimation.hh>
#include <ignition/common/Skeleton.hh>

using namespace ignition;
using namespace common;

/// Private data class
class ignition::common::SkeletonPrivate
{
  typedef std::vector<std::vector<std::pair<std::string, double> > >
    RawNodeWeights;

  /// \brief the root node
  public: SkeletonNode *root;

  /// \brief The dictionary of nodes, indexed by name
  public: SkeletonNodeMap nodes;

  /// \brief the bind pose skeletal transform
  public: math::Matrix4d bindShapeTransform;

  /// \brief the node weight table
  public: RawNodeWeights rawNodeWeights;

  /// \brief the array of animations
  public: std::vector<SkeletonAnimation*> anims;
};

//////////////////////////////////////////////////
Skeleton::Skeleton()
  : data(new SkeletonPrivate)
{
  this->data->root = NULL;
}

//////////////////////////////////////////////////
Skeleton::Skeleton(SkeletonNode *_root)
  : data(new SkeletonPrivate)
{
  this->data->root = _root;
  this->BuildNodeMap();
}

//////////////////////////////////////////////////
Skeleton::~Skeleton()
{
  delete this->data->root;
  delete this->data;
  this->data = NULL;
}

//////////////////////////////////////////////////
void Skeleton::RootNode(SkeletonNode *_node)
{
  this->data->root = _node;
  this->BuildNodeMap();
}

//////////////////////////////////////////////////
SkeletonNode *Skeleton::RootNode() const
{
  return this->data->root;
}

//////////////////////////////////////////////////
SkeletonNode *Skeleton::NodeByName(const std::string &_name) const
{
  for (SkeletonNodeMap::const_iterator iter =
      this->data->nodes.begin(); iter != this->data->nodes.end(); ++iter)
  {
    if (iter->second->Name() == _name)
      return iter->second;
  }

  return NULL;
}

//////////////////////////////////////////////////
SkeletonNode *Skeleton::NodeById(const std::string &_id) const
{
  for (SkeletonNodeMap::const_iterator iter =
      this->data->nodes.begin(); iter != this->data->nodes.end(); ++iter)
  {
    if (iter->second->Id() == _id)
      return iter->second;
  }

  return NULL;
}

//////////////////////////////////////////////////
SkeletonNode *Skeleton::NodeByHandle(const unsigned int _handle) const
{
  SkeletonNodeMap::const_iterator iter = this->data->nodes.find(_handle);
  return iter != this->data->nodes.end() ? iter->second : NULL;
}

//////////////////////////////////////////////////
unsigned int Skeleton::NodeCount() const
{
  return this->data->nodes.size();
}

//////////////////////////////////////////////////
unsigned int Skeleton::JointCount() const
{
  unsigned int c = 0;
  for (SkeletonNodeMap::const_iterator iter =
      this->data->nodes.begin(); iter != this->data->nodes.end(); ++iter)
  {
    if (iter->second->IsJoint())
      c++;
  }

  return c;
}

//////////////////////////////////////////////////
void Skeleton::Scale(const double _scale)
{
  //  scale skeleton structure
  for (SkeletonNodeMap::iterator iter = this->data->nodes.begin();
       iter != this->data->nodes.end(); ++iter)
  {
    SkeletonNode *node = iter->second;
    math::Matrix4d trans = node->Transform();
    math::Vector3d pos = trans.Translation();
    trans.SetTranslation(pos * _scale);
    node->SetTransform(trans, false);
  }

  //  update the nodes' model transforms
  this->data->root->UpdateChildrenTransforms();

  //  scale the animation data
  for (unsigned int i = 0; i < this->data->anims.size(); ++i)
    this->data->anims[i]->Scale(_scale);
}

//////////////////////////////////////////////////
void Skeleton::BuildNodeMap()
{
  std::list<SkeletonNode*> toVisit;
  toVisit.push_front(this->data->root);

  unsigned int handle = 0;

  while (!toVisit.empty())
  {
    SkeletonNode *node = toVisit.front();
    toVisit.pop_front();

    for (int i = (node->ChildCount() - 1); i >= 0; --i)
      toVisit.push_front(node->Child(i));

    node->Handle(handle);
    this->data->nodes[handle] = node;
    handle++;
  }
}

//////////////////////////////////////////////////
void Skeleton::SetBindShapeTransform(const math::Matrix4d &_trans)
{
  this->data->bindShapeTransform = _trans;
}

//////////////////////////////////////////////////
math::Matrix4d Skeleton::BindShapeTransform() const
{
  return this->data->bindShapeTransform;
}

//////////////////////////////////////////////////
void Skeleton::PrintTransforms() const
{
  for (SkeletonNodeMap::const_iterator iter =
      this->data->nodes.begin(); iter != this->data->nodes.end(); ++iter)
  {
    const SkeletonNode *node = iter->second;
    std::cout << "---------------\n" << node->Name() << "\n";

    for (unsigned int i = 0; i < node->RawTransformCount(); ++i)
    {
      NodeTransform nt = node->RawTransform(i);
      std::cout << "\t" << nt.SID();
      if (nt.Type() == NodeTransformType::MATRIX)
      {
        std::cout << " MATRIX\n";
      }
      else
      {
        if (nt.Type() == NodeTransformType::TRANSLATE)
        {
          std::cout << " TRANSLATE\n";
        }
        else
        {
          if (nt.Type() == NodeTransformType::ROTATE)
            std::cout << " ROTATE\n";
          else
            std::cout << " SCALE\n";
        }
      }
      std::cout << nt() << "\n+++++++++++\n";
    }

    std::cout << node->ModelTransform() << "\n";

    if (node->IsJoint())
      std::cout << node->InverseBindTransform() << "\n";
  }
}

//////////////////////////////////////////////////
const SkeletonNodeMap &Skeleton::Nodes() const
{
  return this->data->nodes;
}

//////////////////////////////////////////////////
void Skeleton::SetNumVertAttached(const unsigned int _vertices)
{
  this->data->rawNodeWeights.resize(_vertices);
}

//////////////////////////////////////////////////
void Skeleton::AddVertNodeWeight(
    const unsigned int _vertex, const std::string &_node,
    const double _weight)
{
  if (_vertex < this->data->rawNodeWeights.size())
  {
    this->data->rawNodeWeights[_vertex].push_back(
        std::make_pair(_node, _weight));
  }
}

//////////////////////////////////////////////////
unsigned int Skeleton::VertNodeWeightCount(const unsigned int _vertex) const
{
  if (_vertex < this->data->rawNodeWeights.size())
    return this->data->rawNodeWeights[_vertex].size();
  else
    return 0;
}

//////////////////////////////////////////////////
std::pair<std::string, double> Skeleton::VertNodeWeight(
    const unsigned int _v, const unsigned int _i) const
{
  std::pair<std::string, double> result;

  if (_v < this->data->rawNodeWeights.size() &&
      _i < this->data->rawNodeWeights[_v].size())
  {
    result = this->data->rawNodeWeights[_v][_i];
  }

  return result;
}

//////////////////////////////////////////////////
unsigned int Skeleton::AnimationCount() const
{
  return this->data->anims.size();
}

//////////////////////////////////////////////////
SkeletonAnimation *Skeleton::Animation(const unsigned int _i) const
{
  if (_i < this->data->anims.size())
    return this->data->anims[_i];
  else
    return NULL;
}

//////////////////////////////////////////////////
void Skeleton::AddAnimation(SkeletonAnimation *_anim)
{
  this->data->anims.push_back(_anim);
}
