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
#include <ignition/common/KeyFrame.hh>

using namespace ignition;
using namespace common;

/////////////////////////////////////////////////
KeyFrame::KeyFrame(const double _time)
: time(_time)
{
}

/////////////////////////////////////////////////
KeyFrame::~KeyFrame()
{
}

/////////////////////////////////////////////////
double KeyFrame::Time() const
{
  return this->time;
}


/////////////////////////////////////////////////
PoseKeyFrame::PoseKeyFrame(const double _time)
: KeyFrame(_time)
{
}

/////////////////////////////////////////////////
PoseKeyFrame::~PoseKeyFrame()
{
}

/////////////////////////////////////////////////
void PoseKeyFrame::Translation(const math::Vector3d &_trans)
{
  this->translate = _trans;
}

/////////////////////////////////////////////////
const math::Vector3d &PoseKeyFrame::Translation() const
{
  return this->translate;
}

/////////////////////////////////////////////////
void PoseKeyFrame::Rotation(const math::Quaterniond &_rot)
{
  this->rotate = _rot;
}

/////////////////////////////////////////////////
const math::Quaterniond &PoseKeyFrame::Rotation() const
{
  return this->rotate;
}

/////////////////////////////////////////////////
NumericKeyFrame::NumericKeyFrame(const double _time)
: KeyFrame(_time)
{
}

/////////////////////////////////////////////////
NumericKeyFrame::~NumericKeyFrame()
{
}

/////////////////////////////////////////////////
void NumericKeyFrame::Value(const double &_value)
{
  this->value = _value;
}

/////////////////////////////////////////////////
const double &NumericKeyFrame::Value() const
{
  return this->value;
}
