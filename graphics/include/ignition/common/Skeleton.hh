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
#ifndef IGNITION_COMMON_SKELETON_HH_
#define IGNITION_COMMON_SKELETON_HH_

#include <vector>
#include <string>
#include <map>
#include <utility>

#include <ignition/math/Matrix4.hh>
#include <ignition/common/SkeletonNode.hh>
#include <ignition/common/graphics/Export.hh>

namespace ignition
{
  namespace common
  {
    /// Forward declarations
    class SkeletonPrivate;
    class SkeletonAnimation;

    /// \class Skeleton Skeleton.hh ignition/common/Skeleton.hh
    /// \brief A skeleton, usually used for animation purposes.
    class IGNITION_COMMON_GRAPHICS_VISIBLE Skeleton
    {
      /// \brief Constructor
      public: Skeleton();

      /// \brief Constructor
      /// \param[in] _root node
      public: explicit Skeleton(SkeletonNode *_root);

      /// \brief Destructor
      public: virtual ~Skeleton();

      /// \brief Change the root node
      /// \param[in] _node the new node
      public: void RootNode(SkeletonNode *_node);

      /// \brief Return the root
      /// \return the root
      public: SkeletonNode *RootNode() const;

      /// \brief Find a node
      /// \param[in] _name the name of the node to look for
      /// \return the node, or NULL if not found
      public: SkeletonNode *NodeByName(const std::string &_name) const;

      /// \brief Find node by index
      /// \param[in] _id the index
      /// \return the node, or NULL if not found
      public: SkeletonNode *NodeById(const std::string &_id) const;

      /// \brief Find or create node with handle
      /// \param[in] _handle
      /// \return the node. A new node is created if it didn't exist
      public: SkeletonNode *NodeByHandle(const unsigned int _handle) const;

      /// \brief Returns the node count
      /// \return the count
      public: unsigned int NodeCount() const;

      /// \brief Returns the number of joints
      /// \return the count
      public: unsigned int JointCount() const;

      /// \brief Scale all nodes, transforms and animation data
      /// \param[in] the scaling factor
      public: void Scale(const double _scale);

      /// \brief Set the bind pose skeletal transform
      /// \param[in] _trans the transform
      public: void SetBindShapeTransform(const math::Matrix4d &_trans);

      /// \brief Return bind pose skeletal transform
      /// \return A matrix
      public: math::Matrix4d BindShapeTransform() const;

      /// \brief Outputs the transforms to standard out
      public: void PrintTransforms() const;

      /// \brief Get a copy or the node dictionary.
      /// \return A copy of the nodes.
      public: const SkeletonNodeMap &Nodes() const;

      /// \brief Resizes the raw node weight array
      /// \param[in] _vertices the new size
      public: void SetNumVertAttached(const unsigned int _vertices);

      /// \brief Add a new weight to a node (bone)
      /// \param[in] _vertex index of the vertex
      /// \param[in] _node name of the bone
      /// \param[in] _weight the new weight (range 0 to 1)
      public: void AddVertNodeWeight(const unsigned int _vertex,
                  const std::string &_node, const double _weight);

      /// \brief Returns the number of bone weights for a vertex
      /// \param[in] _vertex the index of the vertex
      /// \return the count
      public: unsigned int VertNodeWeightCount(
                  const unsigned int _vertex) const;

      /// \brief Weight of a bone for a vertex
      /// \param[in] _v the index of the vertex
      /// \param[in] _i the index of the weight for that vertex
      /// \return a pair containing the name of the node and the weight
      public: std::pair<std::string, double> VertNodeWeight(
                  const unsigned int _v, const unsigned int _i) const;

      /// \brief Returns the number of animations
      /// \return the count
      public: unsigned int AnimationCount() const;

      /// \brief Find animation
      /// \param[in] _i the animation index
      /// \return the animation, or NULL if _i is out of bounds
      public: SkeletonAnimation *Animation(const unsigned int _i) const;

      /// \brief Add an animation. The skeleton does not take ownership of the
      /// animation
      /// \param[in] _anim the animation to add
      public: void AddAnimation(SkeletonAnimation *_anim);

      /// \brief Initializes the hande numbers for each node in the map
      /// using breadth first traversal
      private: void BuildNodeMap();

      /// \brief Private data pointer
      private: SkeletonPrivate *data;
    };
  }
}
#endif
