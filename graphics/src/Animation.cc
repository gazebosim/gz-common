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
#include <unordered_map>

#include <gz/math/Spline.hh>
#include <gz/math/Vector2.hh>
#include <gz/math/RotationSpline.hh>
#include <gz/common/Console.hh>
#include <gz/common/KeyFrame.hh>
#include <gz/common/Animation.hh>

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
namespace ignition {
namespace common {
class AnimationPrivate
{
  /// \brief true if the animation is interpolated in x
  public: bool interpolateX = false;
};
}  // namespace ignition
}  // namespace common

// TODO(luca) Make Animation class follow PIMPL and remove global static map
/////////////////////////////////////////////////
typedef std::unordered_map<Animation *, AnimationPrivate *> AnimationPrivateMap;
static AnimationPrivateMap animationPrivateMap;

/////////////////////////////////////////////////
static AnimationPrivate* AnimationDataPtr(Animation *_animation)
{
  auto mapIt = animationPrivateMap.find(_animation);
  if (mapIt == animationPrivateMap.end())
  {
    // Create the map entry
    animationPrivateMap[_animation] = new AnimationPrivate;
  }
  return animationPrivateMap[_animation];
}

/////////////////////////////////////////////////
static void DeleteAnimationDataPtr(Animation *_animation)
{
  // Delete the data pointer class and erase the hash map entry
  auto mapIt = animationPrivateMap.find(_animation);
  if (mapIt != animationPrivateMap.end())
  {
    delete mapIt->second;
    animationPrivateMap.erase(mapIt);
  }
}

/////////////////////////////////////////////////
class gz::common::TrajectoryInfoPrivate
{
  /// \brief ID of the trajectory
  public: unsigned int id{0};

  /// \brief Type of trajectory. If it matches the name a skeleton
  /// animation, they will be played together
  public: unsigned int animIndex{0};

  /// \brief Start time of the trajectory.
  public: std::chrono::steady_clock::time_point startTime;

  /// \brief End time of the trajectory.
  public: std::chrono::steady_clock::time_point endTime;

  /// \brief True if the trajectory is translated.
  public: bool translated{false};

  /// \brief Waypoints in pose animation format.
  /// Times within the animation should be considered durations counted
  /// from start time.
  public: std::shared_ptr<PoseAnimation> waypoints;

  /// \brief Distance on the XY plane covered by each segment. The key is the
  /// duration from start time, the value is the distance in meters.
  public: std::map<std::chrono::steady_clock::duration, double> segDistance;
};

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
  DeleteAnimationDataPtr(this);
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
bool Animation::InterpolateX() const
{
  return AnimationDataPtr(const_cast<Animation *>(this))->interpolateX;
}

/////////////////////////////////////////////////
void Animation::SetInterpolateX(const bool _interpolateX)
{
  AnimationDataPtr(this)->interpolateX = _interpolateX;
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
PoseAnimation::PoseAnimation(const std::string &_name, const double _length,
    const bool _loop)
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
  for (auto kf : this->keyFrames)
    delete kf;
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
  for (auto kf : this->keyFrames)
    delete kf;
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

/////////////////////////////////////////////////
TrajectoryInfo::TrajectoryInfo()
  : dataPtr(new TrajectoryInfoPrivate)
{
}

/////////////////////////////////////////////////
TrajectoryInfo::TrajectoryInfo(TrajectoryInfo &&_trajInfo) noexcept
{
  this->dataPtr = _trajInfo.dataPtr;
  _trajInfo.dataPtr = nullptr;
}

/////////////////////////////////////////////////
TrajectoryInfo::~TrajectoryInfo()
{
  delete this->dataPtr;
}

//////////////////////////////////////////////////
void TrajectoryInfo::CopyFrom(const TrajectoryInfo &_trajInfo)
{
  this->dataPtr->id = _trajInfo.dataPtr->id;
  this->dataPtr->animIndex = _trajInfo.dataPtr->animIndex;
  this->dataPtr->startTime = _trajInfo.dataPtr->startTime;
  this->dataPtr->endTime = _trajInfo.dataPtr->endTime;
  this->dataPtr->translated = _trajInfo.dataPtr->translated;
  this->dataPtr->waypoints = _trajInfo.dataPtr->waypoints;
  this->dataPtr->segDistance = _trajInfo.dataPtr->segDistance;
}

//////////////////////////////////////////////////
TrajectoryInfo::TrajectoryInfo(const TrajectoryInfo &_trajInfo)
  : dataPtr(new TrajectoryInfoPrivate)
{
  this->CopyFrom(_trajInfo);
}

//////////////////////////////////////////////////
TrajectoryInfo &TrajectoryInfo::operator=(const TrajectoryInfo &_trajInfo)
{
  this->CopyFrom(_trajInfo);
  return *this;
}

/////////////////////////////////////////////////
unsigned int TrajectoryInfo::Id() const
{
  return this->dataPtr->id;
}

/////////////////////////////////////////////////
void TrajectoryInfo::SetId(unsigned int _id)
{
  this->dataPtr->id = _id;
}

/////////////////////////////////////////////////
unsigned int TrajectoryInfo::AnimIndex() const
{
  return this->dataPtr->animIndex;
}

/////////////////////////////////////////////////
void TrajectoryInfo::SetAnimIndex(unsigned int _index)
{
  this->dataPtr->animIndex = _index;
}

/////////////////////////////////////////////////
std::chrono::steady_clock::duration TrajectoryInfo::Duration() const
{
  return this->dataPtr->endTime - this->dataPtr->startTime;
}

/////////////////////////////////////////////////
double TrajectoryInfo::DistanceSoFar(
    const std::chrono::steady_clock::duration &_time) const
{
  double distance = 0.0;

  // Sum all completed segments
  auto segment = this->dataPtr->segDistance.begin();
  auto prevSegment = segment;
  for (; segment != this->dataPtr->segDistance.end() && segment->first <= _time;
      prevSegment = segment, ++segment)
  {
    distance += segment->second;
  }

  if (segment == this->dataPtr->segDistance.end())
    return distance;

  // Add difference
  if (_time - prevSegment->first > std::chrono::steady_clock::duration())
  {
    distance += static_cast<double>((_time - prevSegment->first).count()) /
        static_cast<double>((segment->first - prevSegment->first).count()) *
        segment->second;
  }
  return distance;
}

/////////////////////////////////////////////////
std::chrono::steady_clock::time_point TrajectoryInfo::StartTime() const
{
  return this->dataPtr->startTime;
}

/////////////////////////////////////////////////
void TrajectoryInfo::SetStartTime(
    const std::chrono::steady_clock::time_point &_startTime)
{
  this->dataPtr->startTime = _startTime;
}

/////////////////////////////////////////////////
std::chrono::steady_clock::time_point TrajectoryInfo::EndTime() const
{
  return this->dataPtr->endTime;
}

/////////////////////////////////////////////////
void TrajectoryInfo::SetEndTime(
    const std::chrono::steady_clock::time_point &_endTime)
{
  this->dataPtr->endTime = _endTime;
}

/////////////////////////////////////////////////
bool TrajectoryInfo::Translated() const
{
  return this->dataPtr->translated;
}

/////////////////////////////////////////////////
void TrajectoryInfo::SetTranslated(bool _translated)
{
  this->dataPtr->translated = _translated;
}

/////////////////////////////////////////////////
PoseAnimation *TrajectoryInfo::Waypoints() const
{
  return this->dataPtr->waypoints.get();
}

/////////////////////////////////////////////////
void TrajectoryInfo::SetWaypoints(
    std::map<std::chrono::steady_clock::time_point, math::Pose3d> _waypoints)
{
  this->dataPtr->segDistance.clear();

  auto first = _waypoints.begin();
  auto last = _waypoints.rbegin();

  this->SetStartTime(first->first);
  this->SetEndTime(last->first);

  std::stringstream animName;
  animName << this->AnimIndex() << "_" << this->Id();
  std::shared_ptr<PoseAnimation> anim =
      std::make_shared<PoseAnimation>(animName.str(),
      std::chrono::duration<double>(this->Duration()).count(), false);

  auto prevPose = first->second.Pos();
  for (auto pIter = _waypoints.begin(); pIter != _waypoints.end(); ++pIter)
  {
    auto key = anim->CreateKeyFrame(
      std::chrono::duration<double>(pIter->first - this->StartTime()).count());

    math::Vector2d p1(prevPose.X(), prevPose.Y());
    math::Vector2d p2(pIter->second.Pos().X(), pIter->second.Pos().Y());
    this->dataPtr->segDistance[pIter->first - this->StartTime()] =
        p1.Distance(p2);

    key->Translation(pIter->second.Pos());
    key->Rotation(pIter->second.Rot());

    prevPose = pIter->second.Pos();
  }

  this->dataPtr->waypoints = anim;
  this->dataPtr->translated = true;
}
