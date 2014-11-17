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
#ifndef _IGNITION_COMMON_SKELETONANIMATION_PRIVATE_HH_
#define _IGNITION_COMMON_SKELETONANIMATION_PRIVATE_HH_

#include <string>
#include <ignition/common/NodeAnimation.hh>

namespace ignition
{
  namespace common
  {
    class SkeletonAnimationPrivate
    {
      /// \brief the node name
      protected: std::string name;

      /// \brief the duration of the longest animation
      protected: double length;

      /// \brief a dictionary of node animations
      protected: std::map<std::string, NodeAnimation*> animations;
    };
  }
}
#endif
