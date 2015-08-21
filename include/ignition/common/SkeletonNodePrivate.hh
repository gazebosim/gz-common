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
#ifndef _IGNITION_COMMON_SKELETON_NODE_PRIVATE_HH_
#define _IGNITION_COMMON_SKELETON_NODE_PRIVATE_HH_

#include <string>
#include <vector>

#include <ignition/common/SkeletonNode.hh>

namespace ignition
{
  namespace common
  {
    /// \internal
    /// \brief SkeletonNode private data
    class SkeletonNodePrivate
    {
      /// \brief the name of the skeletal node
      public: std::string name;

      /// \brief a string identifier
      public: std::string id;

      /// \brief the type fo node
      public: SkeletonNode::SkeletonNodeType type;

      /// \brief the transform
      public: math::Matrix4d transform;

      /// \brief the initial transformation
      public: math::Matrix4d initialTransform;

      /// \brief the model transformation
      public: math::Matrix4d modelTransform;

      /// \brief the inverse of the bind pose skeletal transform
      public: math::Matrix4d invBindTransform;

      /// \brief the parent node
      public: SkeletonNode *parent;

      /// \brief the children nodes
      public: std::vector<SkeletonNode*> children;

      /// \brief handle index number
      public: unsigned int handle;

      /// \brief the raw transformation
      public: std::vector<NodeTransform> rawTransforms;
    };
  }
}
#endif
