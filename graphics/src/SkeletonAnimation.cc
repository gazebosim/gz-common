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

#include "ignition/common/Console.hh"
#include "ignition/common/NodeAnimation.hh"
#include "ignition/common/SkeletonAnimation.hh"

using namespace ignition;
using namespace common;

/// Prvate data class
class ignition::common::SkeletonAnimationPrivate
{
  /// \brief the node name
  public: std::string name;

  /// \brief the duration of the longest animation
  public: double length;

  /// \brief a dictionary of node animations
  public: std::map<std::string, NodeAnimation*> animations;
};

//////////////////////////////////////////////////
SkeletonAnimation::SkeletonAnimation(const std::string &_name)
  : data(new SkeletonAnimationPrivate)
{
  this->data->name = _name;
}

//////////////////////////////////////////////////
SkeletonAnimation::~SkeletonAnimation()
{
  this->data->animations.clear();
  delete this->data;
  this->data = NULL;
}

//////////////////////////////////////////////////
void SkeletonAnimation::SetName(const std::string &_name)
{
  this->data->name = _name;
}

//////////////////////////////////////////////////
std::string SkeletonAnimation::Name() const
{
  return this->data->name;
}

//////////////////////////////////////////////////
unsigned int SkeletonAnimation::NodeCount() const
{
  return this->data->animations.size();
}

//////////////////////////////////////////////////
bool SkeletonAnimation::HasNode(const std::string &_node) const
{
  return (this->data->animations.find(_node) != this->data->animations.end());
}

//////////////////////////////////////////////////
void SkeletonAnimation::AddKeyFrame(const std::string &_node,
    const double _time, const math::Matrix4d &_mat)
{
  if (this->data->animations.find(_node) == this->data->animations.end())
    this->data->animations[_node] = new NodeAnimation(_node);

  if (_time > this->data->length)
    this->data->length = _time;

  this->data->animations[_node]->AddKeyFrame(_time, _mat);
}

//////////////////////////////////////////////////
void SkeletonAnimation::AddKeyFrame(const std::string &_node,
      const double _time, const math::Pose3d &_pose)
{
  if (this->data->animations.find(_node) == this->data->animations.end())
    this->data->animations[_node] = new NodeAnimation(_node);

  if (_time > this->data->length)
    this->data->length = _time;

  this->data->animations[_node]->AddKeyFrame(_time, _pose);
}

//////////////////////////////////////////////////
math::Matrix4d SkeletonAnimation::NodePoseAt(const std::string &_node,
    const double _time, const bool _loop) const
{
  math::Matrix4d mat;

  if (this->data->animations[_node])
    mat = this->data->animations[_node]->FrameAt(_time, _loop);

  return mat;
}

//////////////////////////////////////////////////
std::map<std::string, math::Matrix4d> SkeletonAnimation::PoseAt(
                      const double _time, const bool _loop) const
{
  ///  TODO need to make sure that all nodes have keyframes at the same
  ///  points in time and create the missing keyframes. if the animation
  ///  comes from bvh this is guaranteed, but if it's comming from collada
  ///  it's not guaranteed. fixing this will help not having to find the
  ///  prev and next keyframe for each node at each time step, but rather
  ///  doing it only once per time step.
  std::map<std::string, math::Matrix4d> pose;
  for (std::map<std::string, NodeAnimation*>::const_iterator iter =
          this->data->animations.begin();
          iter != this->data->animations.end(); ++iter)
  {
    pose[iter->first] = iter->second->FrameAt(_time, _loop);
  }

  return pose;
}

//////////////////////////////////////////////////
std::map<std::string, math::Matrix4d> SkeletonAnimation::PoseAtX(
             const double _x, const std::string &_node, const bool _loop) const
{
  std::map<std::string, NodeAnimation*>::const_iterator nodeAnim =
      this->data->animations.find(_node);
  math::Matrix4d lastPos = nodeAnim->second->KeyFrame(
      nodeAnim->second->FrameCount() - 1).second;

  math::Matrix4d firstPos = nodeAnim->second->KeyFrame(0).second;

  double x = _x;
  if (x < firstPos.Translation().X())
    x = firstPos.Translation().X();

  double lastX = lastPos.Translation().X();
  if (x > lastX && !_loop)
    x = lastX;
  while (x > lastX)
    x -= lastX;

  double time = nodeAnim->second->TimeAtX(x);

  return this->PoseAt(time, _loop);
}

//////////////////////////////////////////////////
void SkeletonAnimation::Scale(const double _scale)
{
  for (std::map<std::string, NodeAnimation*>::iterator iter =
       this->data->animations.begin();
       iter != this->data->animations.end(); ++iter)
  {
    iter->second->Scale(_scale);
  }
}

//////////////////////////////////////////////////
double SkeletonAnimation::Length() const
{
  return this->data->length;
}
