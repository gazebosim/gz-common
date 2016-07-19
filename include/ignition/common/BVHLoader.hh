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
#ifndef IGNITION_COMMON_BVHLOADER_HH_
#define IGNITION_COMMON_BVHLOADER_HH_

#include <vector>
#include <map>
#include <string>

#include <ignition/math/Pose3.hh>
#include <ignition/common/System.hh>

namespace ignition
{
  namespace common
  {
    class Skeleton;

    /// \class BVHLoader BVHLoader.hh ignition/common/BVHLoader.hh
    /// \brief Handles loading BVH animation files
    class IGNITION_COMMON_VISIBLE BVHLoader
    {
      /// \brief Constructor
      public: BVHLoader();

      /// \brief Desutrctor
      public: ~BVHLoader();

      /// \brief Load a BVH file
      /// \param[in] _filename BVH file to load
      /// \param[in] _scale Scaling factor to apply to the skeleton
      /// \return A pointer to a new Skeleton
      public: Skeleton* Load(const std::string &_filename, const double _scale);
    };
  }
}

#endif
