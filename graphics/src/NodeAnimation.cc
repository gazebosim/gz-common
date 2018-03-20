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
#include <map>
#include "ignition/common/Console.hh"
#include "ignition/common/NodeAnimation.hh"

using namespace ignition;
using namespace common;

/// \brief NodeAnimation private data
class ignition::common::NodeAnimationPrivate
{
  /// \brief the name of the animation
  public: std::string name;

  /// \brief the dictionary of key frames, indexed by time
  public: std::map<double, math::Matrix4d> keyFrames;

  /// \brief the duration of the animations (time of last key frame)
  public: double length;
};

//////////////////////////////////////////////////
NodeAnimation::NodeAnimation(const std::string &_name)
: data(new NodeAnimationPrivate)
{
  this->data->name = _name;
  this->data->length = 0.0;
}

//////////////////////////////////////////////////
NodeAnimation::~NodeAnimation()
{
  this->data->keyFrames.clear();
  delete this->data;
  this->data = NULL;
}

//////////////////////////////////////////////////
void NodeAnimation::SetName(const std::string &_name)
{
  this->data->name = _name;
}

//////////////////////////////////////////////////
std::string NodeAnimation::Name() const
{
  return this->data->name;
}

//////////////////////////////////////////////////
void NodeAnimation::AddKeyFrame(const double _time,
    const math::Matrix4d &_trans)
{
  if (_time > this->data->length)
    this->data->length = _time;

  this->data->keyFrames[_time] = _trans;
}

//////////////////////////////////////////////////
void NodeAnimation::AddKeyFrame(const double _time, const math::Pose3d &_pose)
{
  math::Matrix4d mat(_pose.Rot());
  mat.SetTranslation(_pose.Pos());

  this->AddKeyFrame(_time, mat);
}

//////////////////////////////////////////////////
unsigned int NodeAnimation::FrameCount() const
{
  return this->data->keyFrames.size();
}

//////////////////////////////////////////////////
void NodeAnimation::KeyFrame(const unsigned int _i, double &_time,
        math::Matrix4d &_trans) const
{
  if (_i >= this->data->keyFrames.size())
  {
    ignerr << "Invalid key frame index " << _i << "\n";
    _time = -1.0;
  }
  else
  {
    std::map<double, math::Matrix4d>::const_iterator iter =
      this->data->keyFrames.begin();

    std::advance(iter, _i);

    _time = iter->first;
    _trans = iter->second;
  }
}

//////////////////////////////////////////////////
std::pair<double, math::Matrix4d> NodeAnimation::KeyFrame(
  const unsigned int _i) const
{
  double t;
  math::Matrix4d mat(math::Matrix4d::Identity);
  this->KeyFrame(_i, t, mat);

  return std::make_pair(t, mat);
}

//////////////////////////////////////////////////
double NodeAnimation::Length() const
{
  return this->data->length;
}

//////////////////////////////////////////////////
math::Matrix4d NodeAnimation::FrameAt(double _time, bool _loop) const
{
  double time = _time;
  if (time > this->data->length)
  {
    if (_loop)
    {
      while (time > this->data->length)
        time = time - this->data->length;
    }
    else
    {
      time = this->data->length;
    }
  }

  if (math::equal(time, this->data->length))
    return this->data->keyFrames.rbegin()->second;

  std::map<double, math::Matrix4d>::const_iterator it1 =
    this->data->keyFrames.upper_bound(time);

  if (it1 == this->data->keyFrames.begin() || math::equal(it1->first, time))
    return it1->second;

  std::map<double, math::Matrix4d>::const_iterator it2 = it1--;

  if (it1 == this->data->keyFrames.begin() || math::equal(it1->first, time))
    return it1->second;

  double nextKey = it2->first;
  math::Matrix4d nextTrans = it2->second;
  double prevKey = it1->first;
  math::Matrix4d prevTrans = it2->second;

  double t = (time - prevKey) / (nextKey - prevKey);

  if (t >= 0.0 && t <= 1.0)
  {
    ignerr << "Invalid time range\n";
    return math::Matrix4d();
  }

  math::Vector3d nextPos = nextTrans.Translation();
  math::Vector3d prevPos = prevTrans.Translation();
  math::Vector3d pos = math::Vector3d(
      prevPos.X() + ((nextPos.X() - prevPos.X()) * t),
      prevPos.Y() + ((nextPos.Y() - prevPos.Y()) * t),
      prevPos.Z() + ((nextPos.Z() - prevPos.Z()) * t));

  math::Quaterniond nextRot = nextTrans.Rotation();
  math::Quaterniond prevRot = prevTrans.Rotation();
  math::Quaterniond rot = math::Quaterniond::Slerp(t, prevRot, nextRot, true);

  math::Matrix4d trans(rot);
  trans.SetTranslation(pos);

  return trans;
}

//////////////////////////////////////////////////
void NodeAnimation::Scale(const double _scale)
{
  for (std::map<double, math::Matrix4d>::iterator iter =
      this->data->keyFrames.begin();
      iter != this->data->keyFrames.end(); ++iter)
  {
    math::Matrix4d *mat = &iter->second;
    math::Vector3d pos = mat->Translation();
    mat->SetTranslation(pos * _scale);
  }
}

//////////////////////////////////////////////////
double NodeAnimation::TimeAtX(const double _x) const
{
  std::map<double, math::Matrix4d>::const_iterator it1 =
    this->data->keyFrames.begin();

  while (it1->second.Translation().X() < _x)
    ++it1;

  if (it1 == this->data->keyFrames.begin() ||
      math::equal(it1->second.Translation().X(), _x))
  {
    return it1->first;
  }

  std::map<double, math::Matrix4d>::const_iterator it2 = it1--;
  double x1 = it1->second.Translation().X();
  double x2 = it2->second.Translation().X();
  double t1 = it1->first;
  double t2 = it2->first;

  return t1 + ((t2 - t1) * (_x - x1) / (x2 - x1));
}


