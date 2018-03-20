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
#include <algorithm>

#include <ignition/math/Spline.hh>
#include <ignition/math/RotationSpline.hh>
#include <ignition/common/Console.hh>
#include <ignition/common/KeyFrame.hh>
#include <ignition/common/Animation.hh>

using namespace ignition;
using namespace common;

namespace
{
  struct KeyFrameTimeLess
  {
    bool operator() (const common::KeyFrame *_kf,
        const common::KeyFrame *_kf2) const
    {
      return _kf->Time() < _kf2->Time();
    }
  };
}


/////////////////////////////////////////////////
Animation::Animation(const std::string &_name, const double _length,
    const bool _loop)
: name(_name), length(_length), loop(_loop)
{
  this->timePos = 0;
  this->build = false;
}

/////////////////////////////////////////////////
Animation::~Animation()
{
}

/////////////////////////////////////////////////
double Animation::Length() const
{
  return this->length;
}

/////////////////////////////////////////////////
void Animation::Length(const double _len)
{
  this->length = _len;
}

/////////////////////////////////////////////////
void Animation::Time(const double _time)
{
  if (!math::equal(_time, this->timePos))
  {
    this->timePos = _time;
    if (this->loop)
    {
      this->timePos = fmod(this->timePos, this->length);
      if (this->timePos < 0)
        this->timePos += this->length;
    }
    else
    {
      if (this->timePos < 0)
        this->timePos = 0;
      else if (this->timePos > this->length)
        this->timePos = this->length;
    }
  }
}

/////////////////////////////////////////////////
void Animation::AddTime(const double _time)
{
  this->Time(this->timePos + _time);
}

/////////////////////////////////////////////////
double Animation::Time() const
{
  return this->timePos;
}

/////////////////////////////////////////////////
unsigned int Animation::KeyFrameCount() const
{
  return this->keyFrames.size();
}

/////////////////////////////////////////////////
common::KeyFrame *Animation::KeyFrame(const unsigned int _index) const
{
  common::KeyFrame *result = NULL;

  if (_index < this->keyFrames.size())
    result = this->keyFrames[_index];
  else
  {
    ignerr << "Key frame index[" << _index
          << "] is larger than key frame array size["
          << this->keyFrames.size() << "]\n";
  }

  return result;
}

/////////////////////////////////////////////////
double Animation::KeyFramesAtTime(double _time, common::KeyFrame **_kf1,
    common::KeyFrame **_kf2, unsigned int &_firstKeyIndex) const
{
  // Parametric time
  // t1 = time of previous keyframe
  // t2 = time of next keyframe
  double t1, t2;

  // Find first key frame after or on current time
  while (_time > this->length && this->length > 0.0)
    _time -= this->length;

  KeyFrame_V::const_iterator iter;
  common::KeyFrame timeKey(_time);
  iter = std::lower_bound(this->keyFrames.begin(), this->keyFrames.end(),
      &timeKey, KeyFrameTimeLess());

  if (iter == this->keyFrames.end())
  {
    // There is no keyframe after this time, wrap back to first
    *_kf2 = this->keyFrames.front();
    t2 = this->length + (*_kf2)->Time();

    // Use the last keyframe as the previous keyframe
    --iter;
  }
  else
  {
    *_kf2 = *iter;
    t2 = (*_kf2)->Time();

    // Find last keyframe before or on current time
    if (iter != this->keyFrames.begin() && _time < (*iter)->Time())
      --iter;
  }

  _firstKeyIndex = std::distance(this->keyFrames.begin(), iter);

  *_kf1 = *iter;
  t1 = (*_kf1)->Time();

  if (math::equal(t1, t2))
    return 0.0;
  else
    return (_time - t1) / (t2 - t1);
}

/////////////////////////////////////////////////
PoseAnimation::PoseAnimation(const std::string &_name,
    const double _length, const bool _loop)
: Animation(_name, _length, _loop)
{
  this->positionSpline = NULL;
  this->rotationSpline = NULL;
}

/////////////////////////////////////////////////
PoseAnimation::~PoseAnimation()
{
  delete this->positionSpline;
  delete this->rotationSpline;
}

/////////////////////////////////////////////////
PoseKeyFrame *PoseAnimation::CreateKeyFrame(const double _time)
{
  PoseKeyFrame *frame = new PoseKeyFrame(_time);
  std::vector<common::KeyFrame*>::iterator iter =
    std::upper_bound(this->keyFrames.begin(), this->keyFrames.end(),
        reinterpret_cast<common::KeyFrame*>(frame), KeyFrameTimeLess());

  this->keyFrames.insert(iter, frame);
  this->build = true;

  return frame;
}

/////////////////////////////////////////////////
void PoseAnimation::BuildInterpolationSplines() const
{
  if (!this->positionSpline)
    this->positionSpline = new math::Spline();

  if (!this->rotationSpline)
    this->rotationSpline = new math::RotationSpline();

  this->positionSpline->AutoCalculate(false);
  this->rotationSpline->AutoCalculate(false);

  this->positionSpline->Clear();
  this->rotationSpline->Clear();

  for (KeyFrame_V::const_iterator iter = this->keyFrames.begin();
      iter != this->keyFrames.end(); ++iter)
  {
    PoseKeyFrame *pkey = reinterpret_cast<PoseKeyFrame*>(*iter);
    this->positionSpline->AddPoint(pkey->Translation());
    this->rotationSpline->AddPoint(pkey->Rotation());
  }

  this->positionSpline->RecalcTangents();
  this->rotationSpline->RecalcTangents();
  this->build = false;
}

/////////////////////////////////////////////////
void PoseAnimation::InterpolatedKeyFrame(PoseKeyFrame &_kf) const
{
  this->InterpolatedKeyFrame(this->timePos, _kf);
}

/////////////////////////////////////////////////
void PoseAnimation::InterpolatedKeyFrame(const double _time,
                                         PoseKeyFrame &_kf) const
{
  common::KeyFrame *kBase1, *kBase2;
  PoseKeyFrame *k1;
  unsigned int firstKeyIndex;

  if (this->build)
    this->BuildInterpolationSplines();

  double t = this->KeyFramesAtTime(_time, &kBase1, &kBase2, firstKeyIndex);

  k1 = reinterpret_cast<PoseKeyFrame*>(kBase1);

  if (math::equal(t, 0.0))
  {
    _kf.Translation(k1->Translation());
    _kf.Rotation(k1->Rotation());
  }
  else
  {
    _kf.Translation(this->positionSpline->Interpolate(firstKeyIndex, t));
    _kf.Rotation(this->rotationSpline->Interpolate(firstKeyIndex, t));
  }
}

/////////////////////////////////////////////////
NumericAnimation::NumericAnimation(const std::string &_name,
    const double _length, const bool _loop)
: Animation(_name, _length, _loop)
{
}

/////////////////////////////////////////////////
NumericAnimation::~NumericAnimation()
{
}

/////////////////////////////////////////////////
NumericKeyFrame *NumericAnimation::CreateKeyFrame(const double _time)
{
  NumericKeyFrame *frame = new NumericKeyFrame(_time);
  std::vector<common::KeyFrame*>::iterator iter =
    std::upper_bound(this->keyFrames.begin(), this->keyFrames.end(),
        reinterpret_cast<common::KeyFrame*>(frame),
        KeyFrameTimeLess());

  this->keyFrames.insert(iter, frame);
  this->build = true;
  return frame;
}

/////////////////////////////////////////////////
void NumericAnimation::InterpolatedKeyFrame(NumericKeyFrame &_kf) const
{
  // Keyframe pointers
  common::KeyFrame *kBase1, *kBase2;
  NumericKeyFrame *k1, *k2;
  unsigned int firstKeyIndex;

  double t;
  t = this->KeyFramesAtTime(this->timePos, &kBase1, &kBase2, firstKeyIndex);

  k1 = reinterpret_cast<NumericKeyFrame*>(kBase1);
  k2 = reinterpret_cast<NumericKeyFrame*>(kBase2);

  if (math::equal(t, 0.0))
  {
    // Just use k1
    _kf.Value(k1->Value());
  }
  else
  {
    // Interpolate by t
    double diff = k2->Value() - k1->Value();
    _kf.Value(k1->Value() + diff * t);
  }
}
