/*
 * Copyright (C) 2019 Open Source Robotics Foundation
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

#include <gtest/gtest.h>

#include "test_config.h"
#include "ignition/common/ColladaLoader.hh"
#include "ignition/common/Material.hh"
#include "ignition/common/Mesh.hh"
#include "ignition/common/Skeleton.hh"
#include "ignition/common/SubMesh.hh"
#include "ignition/common/MeshManager.hh"
#include "ignition/common/SystemPaths.hh"
#include "ignition/math/Vector3.hh"
#include "test/util.hh"

class SkeletonTest : public ignition::testing::AutoLogFixture { };

using namespace ignition;

TEST_F(SkeletonTest, DoubleSkeletonMerge)
{
  common::ColladaLoader loader;
  common::Mesh *mesh = loader.Load(std::string(PROJECT_SOURCE_PATH) +
     "/test/data/box_with_double_skeleton.dae");
  EXPECT_TRUE(mesh->HasSkeleton());
  auto skeleton_ptr = mesh->MeshSkeleton();
  // The two skeletons have been joined and their root is the
  // animation root, called Armature
  EXPECT_EQ(skeleton_ptr->RootNode()->Name(), std::string("Armature"));
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
