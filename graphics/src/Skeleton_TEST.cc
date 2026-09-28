/*
 * Copyright (C) 2026 Open Source Robotics Foundation
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

#include <gz/common/Mesh.hh>
#include <gz/common/MeshManager.hh>
#include <gz/common/Skeleton.hh>
#include <gz/common/SkeletonAnimation.hh>
#include <gz/common/testing/AutoLogFixture.hh>
#include <gz/common/testing/TestPaths.hh>

using namespace gz;

// Runs the test twice, once each for GZ_MESH_FORCE_ASSIMP=true and false
// to test both BVHLoader and AssimpLoader
class SkeletonTest : public common::testing::AutoLogFixture,
                     public testing::WithParamInterface<bool>
{
  protected: void SetUp() override
  {
    common::testing::AutoLogFixture::SetUp();

    if (this->GetParam())
    {
      common::setenv("GZ_MESH_FORCE_ASSIMP", "true");
    }
    else
    {
      common::setenv("GZ_MESH_FORCE_ASSIMP", "false");
    }
  }

  protected: void TearDown() override
  {
    common::unsetenv("GZ_MESH_FORCE_ASSIMP");
    common::MeshManager::Instance()->RemoveAll();
    common::testing::AutoLogFixture::TearDown();
  }
};

/////////////////////////////////////////////////
TEST_P(SkeletonTest, AddBvhAnimation)
{
  auto *mgr = common::MeshManager::Instance();
  const common::Mesh *mesh = mgr->Load(
      common::testing::TestFile("data", "walk.dae"));
  ASSERT_NE(nullptr, mesh);

  common::SkeletonPtr skel = mesh->MeshSkeleton();
  ASSERT_NE(nullptr, skel);
  ASSERT_EQ(1u, skel->AnimationCount());

  // Non-existent or empty file should fail
  EXPECT_FALSE(skel->AddBvhAnimation("", 1.0));
  EXPECT_FALSE(skel->AddBvhAnimation("nonexistent.bvh", 1.0));
  EXPECT_EQ(1u, skel->AnimationCount());

  // File without a skeleton / invalid BVH should fail
  EXPECT_FALSE(skel->AddBvhAnimation(
      common::testing::TestFile("data", "box.dae"), 1.0));
  EXPECT_EQ(1u, skel->AnimationCount());

  // Incompatible skeleton (node count mismatch) should fail
  const std::string bvhFile =
      common::testing::TestFile("data", "cmu-13_26.bvh");
  common::Skeleton emptySkel;
  EXPECT_FALSE(emptySkel.AddBvhAnimation(bvhFile, 1.0));
  EXPECT_EQ(0u, emptySkel.AnimationCount());

  // Compatible BVH animation should succeed
  EXPECT_TRUE(skel->AddBvhAnimation(bvhFile, 1.0));
  ASSERT_EQ(2u, skel->AnimationCount());

  common::SkeletonAnimation *bvhAnim = skel->Animation(1u);
  ASSERT_NE(nullptr, bvhAnim);
  EXPECT_EQ(bvhFile, bvhAnim->Name());
  EXPECT_EQ(31u, bvhAnim->NodeCount());
  EXPECT_NEAR(25.2332, bvhAnim->Length(), 1e-3);
  EXPECT_EQ(skel->RootNode()->Name(), std::string("Hips"));
}

INSTANTIATE_TEST_SUITE_P(
    ForceAssimpScenarios,
    SkeletonTest,
    testing::Bool());
