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
#ifndef _IGNITION_COMMON_SKELETON_PRIVATE_HH_
#define _IGNITION_COMMON_SKELETON_PRIVATE_HH_

#include <ignition/common/SkeletonAnimation.hh>
#include <ignition/common/SkeletonNode.hh>

namespace ignition
{
  namespace common
  {
    class SkeletonPrivate
    {
      typedef std::vector<std::vector<std::pair<std::string, double>>>
        RawNodeWeights;

      /// \brief the root node
      public: SkeletonNode *root;

      /// \brief The dictionary of nodes, indexed by name
      public: SkeletonNodeMap nodes;

      /// \brief the bind pose skeletal transform
      public: math::Matrix4d bindShapeTransform;

      /// \brief the node weight table
      public: RawNodeWeights rawNodeWeights;

      /// \brief the array of animations
      public: std::vector<SkeletonAnimation*> anims;
    };
  }
}
#endif


