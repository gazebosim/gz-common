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
#ifndef GZ_COMMON_ANIMATION_HH_
#define GZ_COMMON_ANIMATION_HH_

#include <map>
#include <string>
#include <vector>

#include <gz/math/Pose3.hh>
#include <gz/math/Spline.hh>
#include <gz/math/RotationSpline.hh>

#include <gz/common/graphics/Export.hh>

namespace ignition
{
  namespace common
  {
    class KeyFrame;
    class PoseKeyFrame;
    class NumericKeyFrame;
    class TrajectoryInfoPrivate;

    /// \class Animation Animation.hh ignition/common/Animation.hh
    /// \brief Manages an animation, which is a collection of keyframes and
    /// the ability to interpolate between the keyframes
    class IGNITION_COMMON_GRAPHICS_VISIBLE Animation
    {
      /// \brief Constructor
      /// \param[in] _name Name of the animation, should be unique
      /// \param[in] _length Duration of the animation in seconds
      /// \param[in] _loop Set to true if the animation should repeat
      public: Animation(const std::string &_name,
                  const double _length, const bool _loop);

      /// \brief Destructor
      public: virtual ~Animation();

      /// \brief Return the duration of the animation
      /// \return Duration of the animation in seconds
      public: double Length() const;

      /// \brief Set the duration of the animation
      /// \param[in] _len The length of the animation in seconds
      public: void Length(const double _len);

      /// \brief Set the current time position of the animation
      /// \param[in] _time The time position in seconds
      public: void Time(const double _time);

      /// \brief Add time to the animation
      /// \param[in] _time The amount of time to add in seconds
      public: void AddTime(const double _time);

      /// \brief Return the current time position
      /// \return The time position in seconds
      public: double Time() const;

      /// \brief Return the interpolateX value
      /// \return The interpolateX value
      public: bool InterpolateX() const;

      /// \brief Set the interpolateX value
      /// \param[in] _interpolateX The interpolateX value
      public: void SetInterpolateX(const bool _interpolateX);

      /// \brief Return the number of key frames in the animation
      /// \return The number of keyframes
      public: unsigned int KeyFrameCount() const;

      /// \brief Get a key frame using an index value
      /// \param[in] _index The index of the key frame
      /// \return A pointer the keyframe, NULL if the _index is invalid
      public: common::KeyFrame *KeyFrame(const unsigned int _index) const;

      /// \brief Get the two key frames that bound a time value
      /// \param[in] _time The time in seconds
      /// \param[out] _kf1 Lower bound keyframe that is returned
      /// \param[out] _kf2 Upper bound keyframe that is returned
      /// \param[out] _firstKeyIndex Index of the lower bound key frame
      /// \return The time between the two keyframe
      protected: double KeyFramesAtTime(
                     double _time, common::KeyFrame **_kf1,
                     common::KeyFrame **_kf2,
                     unsigned int &_firstKeyIndex) const;

#ifdef _WIN32
// Disable warning C4251 which is triggered by
// std::unique_ptr
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

      /// \brief animation name
      protected: std::string name;

      /// \brief animation duration
      protected: double length;

      /// \brief current time position
      protected: double timePos;

      /// \brief determines if the interpolation splines need building
      protected: mutable bool build;

      /// \brief true if animation repeats
      protected: bool loop;

      /// \brief array of keyframe type alias
      protected: typedef std::vector<common::KeyFrame *> KeyFrame_V;

      /// \brief array of key frames
      protected: KeyFrame_V keyFrames;
#ifdef _WIN32
#pragma warning(pop)
#endif
    };

    /// \brief A pose animation.
    class IGNITION_COMMON_GRAPHICS_VISIBLE PoseAnimation : public Animation
    {
      /// \brief Constructor
      /// \param[in] _name String name of the animation. This should be unique.
      /// \param[in] _length Length of the animation in seconds
      /// \param[in] _loop True == loop the animation
      public: PoseAnimation(const std::string &_name,
                  const double _length, const bool _loop);

      /// \brief Destructor
      public: virtual ~PoseAnimation();

      /// \brief Create a pose keyframe at the given time
      /// \param[in] _time Time at which to create the keyframe
      /// \return Pointer to the new keyframe
      public: PoseKeyFrame *CreateKeyFrame(const double _time);

      /// \brief Get a keyframe using the animation's current time.
      /// \param[out] _kf PoseKeyFrame reference to hold the interpolated result
      public: void InterpolatedKeyFrame(PoseKeyFrame &_kf) const;

      /// \brief Get a keyframe using a passed in time.
      /// \param[in] _time Time in seconds
      /// \param[out] _kf PoseKeyFrame reference to hold the interpolated result
      protected: void InterpolatedKeyFrame(const double _time,
                                              PoseKeyFrame &_kf) const;

      /// \brief Update the pose splines
      protected: void BuildInterpolationSplines() const;

      /// \brief smooth interpolation for position
      private: mutable math::Spline *positionSpline;

      /// \brief smooth interpolation for rotation
      private: mutable math::RotationSpline *rotationSpline;
    };

    /// \brief A numeric animation.
    class IGNITION_COMMON_GRAPHICS_VISIBLE NumericAnimation : public Animation
    {
      /// \brief Constructor
      /// \param[in] _name String name of the animation. This should be unique.
      /// \param[in] _length Length of the animation in seconds
      /// \param[in] _loop True == loop the animation
      public: NumericAnimation(const std::string &_name,
                               const double _length, const bool _loop);

      /// \brief Destructor
      public: virtual ~NumericAnimation();

      /// \brief Create a numeric keyframe at the given time
      /// \param[in] _time Time at which to create the keyframe
      /// \return Pointer to the new keyframe
      public: NumericKeyFrame *CreateKeyFrame(const double _time);

      /// \brief Get a keyframe using the animation's current time.
      /// \param[out] _kf NumericKeyFrame reference to hold the
      /// interpolated result
      public: void InterpolatedKeyFrame(NumericKeyFrame &_kf) const;
    };

    /// \brief Information about a trajectory for an animation (e.g., Actor)
    /// This contains the keyframe information.
    class IGNITION_COMMON_GRAPHICS_VISIBLE TrajectoryInfo
    {
      /// \brief Constructor
      public: TrajectoryInfo();

      /// \brief Copy constructor
      /// \param[in] _trajInfo TrajectoryInfo to copy.
      public: TrajectoryInfo(const TrajectoryInfo &_trajInfo);

      /// \brief Move constructor
      /// \param[in] _trajInfo TrajectoryInfo to move.
      public: TrajectoryInfo(TrajectoryInfo &&_trajInfo) noexcept;

      /// \brief Destructor
      public: ~TrajectoryInfo();

      /// \brief Assignment operator.
      /// \param[in] _trajInfo The TrajectoryInfo to set values from.
      /// \return *this
      public: TrajectoryInfo &operator=(const TrajectoryInfo &_trajInfo);

      /// \brief Copy TrajectoryInfo from a TrajectoryInfo instance.
      /// \param[in] _trajInfo The TrajectoryInfo to set values from.
      public: void CopyFrom(const TrajectoryInfo &_trajInfo);

      /// \brief Return the id of the trajectory
      /// \return Id of the trajectory
      public: unsigned int Id() const;

      /// \brief Set the id of the trajectory
      /// \param[in] _id Id for the trajectory
      public: void SetId(unsigned int _id);

      /// \brief Return the animation index
      /// \return Index of the associated animation
      public: unsigned int AnimIndex() const;

      /// \brief Set the animation index
      /// \param[in] _index Animation index
      /// (auto-generated according to the type)
      public: void SetAnimIndex(unsigned int _index);

      /// \brief Return the duration of the trajectory.
      /// \return Duration of the animation.
      public: std::chrono::steady_clock::duration Duration() const;

      /// \brief Get the distance covered by the trajectory by a given time.
      /// \param[in] _time Time from trajectory start to check the distance.
      /// \return Distance in meters covered by the trajectory.
      public: double DistanceSoFar(
          const std::chrono::steady_clock::duration &_time) const;

      /// \brief Return the start time of the trajectory.
      /// \return Start time of the trajectory.
      public: std::chrono::steady_clock::time_point StartTime() const;

      /// \brief Set the start time of the trajectory.
      /// \param[in] _startTime Trajectory start time.
      public: void SetStartTime(
          const std::chrono::steady_clock::time_point &_startTime);

      /// \brief Return the end time of the trajectory
      /// \return End time of the trajectory in seconds
      public: std::chrono::steady_clock::time_point EndTime() const;

      /// \brief Set the end time of the trajectory.
      /// \param[in] _endTime Trajectory end time.
      public: void SetEndTime(
          const std::chrono::steady_clock::time_point &_endTime);

      /// \brief Return whether the trajectory is translated
      /// \return True if the trajectory is translated
      public: bool Translated() const;

      /// \brief Set whether the trajectory is translated
      /// \param[in] _translated True if the trajectory is translated
      public: void SetTranslated(bool _translated);

      /// \brief Return the waypoints in the trajectory
      /// \return Waypoints represented in pose animation format
      public: common::PoseAnimation *Waypoints() const;

      /// \brief Load all waypoints in the trajectory
      /// \param[in] _waypoints Map of waypoints, where the key is the absolute
      /// time of the waypoint and the value is the pose.
      public: void SetWaypoints(
          std::map<std::chrono::steady_clock::time_point, math::Pose3d>
           _waypoints);

      /// \brief Private data pointer.
      private: TrajectoryInfoPrivate *dataPtr{nullptr};
    };
  }
}
#endif
