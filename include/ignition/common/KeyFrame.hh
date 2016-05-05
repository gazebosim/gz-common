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
#ifndef IGNITION_COMMON_KEYFRAME_HH_
#define IGNITION_COMMON_KEYFRAME_HH_

#include <ignition/math/Vector3.hh>
#include <ignition/math/Quaternion.hh>
#include <ignition/common/System.hh>

namespace ignition
{
  namespace common
  {
    /// \class KeyFrame KeyFrame.hh ignition/common/KeyFrame.hh
    /// \brief A key frame in an animation
    class IGNITION_VISIBLE KeyFrame
    {
      /// \brief Constructor
      /// \param[in] _time Time of the keyframe in seconds
      public: KeyFrame(const double _time);

      /// \brief Destructor
      public: virtual ~KeyFrame();

      /// \brief Get the time of the keyframe
      /// \return the time
      public: double Time() const;

      /// \brief time of key frame
      protected: double time;
    };

    /// \brief A keyframe for a PoseAnimation
    class IGNITION_VISIBLE PoseKeyFrame : public KeyFrame
    {
      /// \brief Constructor
      /// \param[in] _time of the keyframe
      public: PoseKeyFrame(const double _time);

      /// \brief Destructor
      public: virtual ~PoseKeyFrame();

      /// \brief Set the translation for the keyframe
      /// \param[in] _trans Translation amount
      public: void Translation(const math::Vector3d &_trans);

      /// \brief Get the translation of the keyframe
      /// \return The translation amount
      public: const math::Vector3d &Translation() const;

      /// \brief Set the rotation for the keyframe
      /// \param[in] _rot Rotation amount
      public: void Rotation(const math::Quaterniond &_rot);

      /// \brief Get the rotation of the keyframe
      /// \return The rotation amount
      public: const math::Quaterniond &Rotation() const;

      /// \brief the translation vector
      protected: math::Vector3d translate;

      /// \brief the rotation quaternion
      protected: math::Quaterniond rotate;
    };

    /// \brief A keyframe for a NumericAnimation
    class IGNITION_VISIBLE NumericKeyFrame : public KeyFrame
    {
      /// \brief Constructor
      /// \param[in] _time Time of the keyframe
      public: NumericKeyFrame(const double _time);

      /// \brief Destructor
      public: virtual ~NumericKeyFrame();

      /// \brief Set the value of the keyframe
      /// \param[in] _value The new value
      public: void Value(const double &_value);

      /// \brief Get the value of the keyframe
      /// \return the value of the keyframe
      public: const double &Value() const;

      /// \brief numeric value
      protected: double value;
    };
  }
}
#endif
