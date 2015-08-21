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
#ifndef _IGNITION_COMMON_NODE_ANIMATION_PRIVATE_HH_
#define _IGNITION_COMMON_NODE_ANIMATION_PRIVATE_HH_

#include <map>
#include <string>

#include <ignition/math/Matrix4.hh>

namespace ignition
{
  namespace common
  {
    /// \internal
    /// \brief NodeAnimation private data
    class NodeAnimationPrivate
    {
      /// \brief the name of the animation
      public: std::string name;

      /// \brief the dictionary of key frames, indexed by time
      public: std::map<double, math::Matrix4d> keyFrames;

      /// \brief the duration of the animations (time of last key frame)
      public: double length;
    };
  }
}
#endif
