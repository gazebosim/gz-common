/*
 * Copyright (C) 2022 Open Source Robotics Foundation
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

#include "gz/common/Material.hh"
#include "gz/common/Mesh.hh"
#include "gz/common/Skeleton.hh"
#include "gz/common/SkeletonAnimation.hh"
#include "gz/common/SubMesh.hh"
#include "gz/common/AssimpLoader.hh"

#include "gz/common/testing/AutoLogFixture.hh"
#include "gz/common/testing/TestPaths.hh"

using namespace gz;
class AssimpLoader : public common::testing::AutoLogFixture { };

/////////////////////////////////////////////////
TEST_F(AssimpLoader, LoadZeroCount)
{
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "zero_count.dae"));
  ASSERT_TRUE(mesh);
  delete mesh;
}

/////////////////////////////////////////////////
TEST_F(AssimpLoader, LoadBoxInstControllerWithoutSkeleton)
{
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data",
        "box_inst_controller_without_skeleton.dae"));

  EXPECT_EQ(36u, mesh->IndexCount());
  EXPECT_EQ(24u, mesh->VertexCount());
  EXPECT_EQ(1u, mesh->SubMeshCount());
  EXPECT_EQ(1u, mesh->MaterialCount());
  EXPECT_EQ(24u, mesh->TexCoordCount());
  common::SkeletonPtr skeleton = mesh->MeshSkeleton();
  EXPECT_LT(0u, skeleton->NodeCount());
  EXPECT_NE(nullptr, skeleton->NodeById("Armature_Bone"));
  delete mesh;
}

/////////////////////////////////////////////////
TEST_F(AssimpLoader, LoadBoxMultipleInstControllers)
{
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "box_multiple_inst_controllers.dae"));

  EXPECT_EQ(72u, mesh->IndexCount());
  EXPECT_EQ(48u, mesh->VertexCount());
  EXPECT_EQ(2u, mesh->SubMeshCount());
  EXPECT_EQ(1u, mesh->MaterialCount());
  EXPECT_EQ(48u, mesh->TexCoordCount());

  std::shared_ptr<common::SubMesh> submesh = mesh->SubMeshByIndex(0).lock();
  std::shared_ptr<common::SubMesh> submesh2 = mesh->SubMeshByIndex(1).lock();
  EXPECT_EQ(36u, submesh->IndexCount());
  EXPECT_EQ(36u, submesh2->IndexCount());
  EXPECT_EQ(24u, submesh->VertexCount());
  EXPECT_EQ(24u, submesh2->VertexCount());
  EXPECT_EQ(24u, submesh->TexCoordCount());
  EXPECT_EQ(24u, submesh2->TexCoordCount());

  common::SkeletonPtr skeleton = mesh->MeshSkeleton();
  EXPECT_LT(0u, skeleton->NodeCount());
  EXPECT_NE(nullptr, skeleton->NodeById("Armature_Bone"));
  delete mesh;
}

/////////////////////////////////////////////////
TEST_F(AssimpLoader, LoadBoxWithDefaultStride)
{
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "box_with_default_stride.dae"));
  ASSERT_NE(mesh, nullptr);

  EXPECT_EQ(36u, mesh->IndexCount());
  EXPECT_EQ(24u, mesh->VertexCount());
  EXPECT_EQ(1u, mesh->SubMeshCount());
  EXPECT_EQ(1u, mesh->MaterialCount());
  EXPECT_EQ(24u, mesh->TexCoordCount());
  ASSERT_NE(mesh->MeshSkeleton(), nullptr);
  // TODO(luca) not working, investigate
  // ASSERT_EQ(1u, mesh->MeshSkeleton()->AnimationCount());
  delete mesh;
}

/////////////////////////////////////////////////
TEST_F(AssimpLoader, MergeBoxWithDoubleSkeleton)
{
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "box_with_double_skeleton.dae"));
  ASSERT_TRUE(mesh->HasSkeleton());
  auto skeleton_ptr = mesh->MeshSkeleton();
  // The two skeletons have been joined and their root is the
  // animation root, called Scene
  EXPECT_EQ(skeleton_ptr->RootNode()->Name(), std::string("Scene"));
  delete mesh;
}

/////////////////////////////////////////////////
TEST_F(AssimpLoader, LoadCylinderAnimatedFrom3dsMax)
{
  // TODO(anyone) This test shows that the mesh loads without crashing, but the
  // mesh animation looks deformed when loaded. That still needs to be
  // addressed.
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data",
        "cylinder_animated_from_3ds_max.dae"));

  EXPECT_EQ("unknown", mesh->Name());
  EXPECT_EQ(194u, mesh->VertexCount());
  EXPECT_EQ(194u, mesh->NormalCount());
  EXPECT_EQ(852u, mesh->IndexCount());
  EXPECT_LT(0u, mesh->TexCoordCount());
  EXPECT_EQ(0u, mesh->MaterialCount());

  EXPECT_EQ(1u, mesh->SubMeshCount());
  auto subMesh = mesh->SubMeshByIndex(0);
  ASSERT_NE(nullptr, subMesh.lock());
  EXPECT_EQ("Cylinder01", subMesh.lock()->Name());

  EXPECT_TRUE(mesh->HasSkeleton());
  auto skeleton = mesh->MeshSkeleton();
  ASSERT_NE(nullptr, skeleton);
  ASSERT_EQ(1u, skeleton->AnimationCount());

  auto anim = skeleton->Animation(0);
  ASSERT_NE(nullptr, anim);
  // TODO(luca) Fix is merged in assimp main, add when it is re-released
  // EXPECT_EQ("Bone02", anim->Name());
  EXPECT_EQ(1u, anim->NodeCount());
  EXPECT_TRUE(anim->HasNode("Bone02"));
  delete mesh;
}

/////////////////////////////////////////////////
// Checks for null root node animation and valid
// x displacement in non root node's animation.
TEST_F(AssimpLoader, CheckNonRootDisplacement)
{
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(common::testing::TestFile("data",
        "walk.dae"));
  auto meshSkel =  mesh->MeshSkeleton();
  std::string rootNodeName = meshSkel->RootNode()->Name();
  common::SkeletonAnimation *skelAnim = meshSkel->Animation(0);
  common::NodeAnimation *rootNode = skelAnim->NodeAnimationByName(rootNodeName);
  EXPECT_EQ(nullptr, rootNode);
  auto xDisplacement = skelAnim->XDisplacement();
  ASSERT_TRUE(xDisplacement);
  delete mesh;
}
