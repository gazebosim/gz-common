/*
 * Copyright (C) 2014 Open Source Robotics Foundation
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

#include <ignition/common/SkeletonNodePrivate.hh>
#include <ignition/common/SkeletonNode.hh>

using namespace ignition;
using namespace common;

//////////////////////////////////////////////////
SkeletonNode::SkeletonNode(const SkeletonNode *_parent)
  : data(new SkeletonNodePrivate)
{
  this->data->parent = _parent;

  if (this->data->parent)
    this->data->parent->AddChild(this);
}

//////////////////////////////////////////////////
SkeletonNode::SkeletonNode(const SkeletonNode *_parent,
    const std::string &_name, const std::string &_id,
    const SkeletonNodeType _type)
  : data(new SkeletonNodePrivate)
{
  this->data->parent = _parent;

  if (this->data->parent)
    this->data->parent->AddChild(this);

  this->data->name = _name;
  this->data->id = _id;
  this->data->type = _type;
}

//////////////////////////////////////////////////
SkeletonNode::~SkeletonNode()
{
  this->data->children.clear();
  delete this->data;
  this->data = NULL;
}

//////////////////////////////////////////////////
void SkeletonNode::Name(const std::string &_name)
{
  this->data->name = _name;
}

//////////////////////////////////////////////////
std::string SkeletonNode::Name() const
{
  return this->data->name;
}

//////////////////////////////////////////////////
void SkeletonNode::Id(const std::string &_id)
{
  this->data->id = _id;
}

//////////////////////////////////////////////////
std::string SkeletonNode::Id() const
{
  return this->data->id;
}

//////////////////////////////////////////////////
void SkeletonNode::Type(const SkeletonNodeType _type)
{
  this->data->type = _type;
}

//////////////////////////////////////////////////
bool SkeletonNode::IsJoint() const
{
  if (this->data->type == JOINT)
    return true;
  else
    return false;
}

//////////////////////////////////////////////////
void SkeletonNode::Transform(const math::Matrix4d &_trans,
    const bool _updateChildren)
{
  this->data->transform = _trans;

  if (this->data->parent == NULL)
  {
    this->data->modelTransform = _trans;
  }
  else
  {
    this->data->modelTransform =
      this->data->parent->ModelTransform() * _trans;
  }

  /// propagate the change to the children nodes
  if (_updateChildren)
    this->UpdateChildrenTransforms();
}

//////////////////////////////////////////////////
void SkeletonNode::InitialTransform(const math::Matrix4d &_trans)
{
  this->data->initialTransform = _trans;
  this->Transform(_trans);
}

//////////////////////////////////////////////////
void SkeletonNode::Reset(const bool _resetChildren)
{
  this->SetTransform(this->data->initialTransform);

  if (_resetChildren)
  {
    for (unsigned int i = 0; i < this->ChildCount(); ++i)
      this->Child(i)->Reset(true);
  }
}

//////////////////////////////////////////////////
void SkeletonNode::UpdateChildrenTransforms()
{
  std::list<SkeletonNode*> toVisit;
  for (unsigned int i = 0; i < this->data->children.size(); ++i)
    toVisit.push_back(this->data->children[i]);

  while (!toVisit.empty())
  {
    SkeletonNode *node = toVisit.front();
    toVisit.pop_front();

    for (int i = (node->ChildCount() - 1); i >= 0; --i)
      toVisit.push_front(node->Child(i));

    node->modelTransform = node->Parent()->modelTransform * node->transform;
  }
}

//////////////////////////////////////////////////
math::Matrix4d SkeletonNode::Transform() const
{
  return this->data->transform;
}

//////////////////////////////////////////////////
void SkeletonNode::ModelTransform(const math::Matrix4d &_trans,
    const bool _updateChildren)
{
  this->data->modelTransform = _trans;

  if (this->data->parent == NULL)
    this->data->transform = _trans;
  else
  {
    math::Matrix4d invParentTrans =
      this->data->parent->ModelTransform().Inverse();
    this->data->transform = invParentTrans * this->data->modelTransform;
  }

  if (_updateChildren)
    this->UpdateChildrenTransforms();
}

//////////////////////////////////////////////////
math::Matrix4d SkeletonNode::ModelTransform() const
{
  return this->data->modelTransform;
}

//////////////////////////////////////////////////
void SkeletonNode::Parent(const SkeletonNode *_parent)
{
  this->data->parent = _parent;
}

//////////////////////////////////////////////////
SkeletonNode *SkeletonNode::Parent() const
{
  return this->data->parent;
}

//////////////////////////////////////////////////
bool SkeletonNode::IsRootNode() const
{
  if (!this->data->parent)
    return true;
  else
    return false;
}

//////////////////////////////////////////////////
void SkeletonNode::AddChild(const SkeletonNode *_child)
{
  this->data->children.push_back(_child);
}

//////////////////////////////////////////////////
unsigned int SkeletonNode::ChildCount() const
{
  return this->data->children.size();
}

//////////////////////////////////////////////////
SkeletonNode *SkeletonNode::Child(const unsigned int _index) const
{
  SkeletonNode *result = NULL;

  if (_index >= this->data->children.size())
    ignerr << "Index out of range[" << _index << "]\n";
  else
    result = this->data->children[_index];

  return result;
}

//////////////////////////////////////////////////
SkeletonNode *SkeletonNode::ChildByName(const std::string &_name) const
{
  for (unsigned int i = 0; i < this->data->children.size(); ++i)
  {
    if (this->data->children[i]->GetName() == _name)
      return this->data->children[i];
  }

  return NULL;
}

//////////////////////////////////////////////////
SkeletonNode *SkeletonNode::ChildById(const std::string &_id) const
{
  for (unsigned int i = 0; i < this->data->children.size(); ++i)
  {
    if (this->data->children[i]->GetId() == _id)
      return this->data->children[i];
  }

  return NULL;
}

//////////////////////////////////////////////////
void SkeletonNode::Handle(const unsigned int _handle)
{
  this->data->handle = _handle;
}

//////////////////////////////////////////////////
unsigned int SkeletonNode::Handle() const
{
  return this->data->handle;
}

//////////////////////////////////////////////////
void SkeletonNode::InverseBindTransform(const math::Matrix4d &_invBM)
{
  this->data->invBindTransform = _invBM;
}

//////////////////////////////////////////////////
math::Matrix4d SkeletonNode::InverseBindTransform() const
{
  return this->data->invBindTransform;
}

//////////////////////////////////////////////////
std::vector<NodeTransform> SkeletonNode::RawTransforms() const
{
  return this->data->rawTransforms;
}

//////////////////////////////////////////////////
unsigned int SkeletonNode::RawTransformCount() const
{
  return this->data->rawTransforms.size();
}

//////////////////////////////////////////////////
NodeTransform SkeletonNode::GetRawTransform(const unsigned int _i)
{
  return this->data->rawTransforms[_i];
}

//////////////////////////////////////////////////
void SkeletonNode::AddRawTransform(const NodeTransform _t)
{
  this->data->rawTransforms.push_back(_t);
}

//////////////////////////////////////////////////
std::vector<NodeTransform> SkeletonNode::Transforms() const
{
  return this->data->rawTransforms;
}
