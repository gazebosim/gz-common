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
#include <gtest/gtest.h>

#include <memory>

#include "gz/common/Mesh.hh"
#include "gz/common/SubMesh.hh"
#include "gz/common/Material.hh"
#include "gz/common/ColladaLoader.hh"
#include "gz/common/Skeleton.hh"
#include "gz/common/SkeletonAnimation.hh"

#include "gz/common/testing/AutoLogFixture.hh"
#include "gz/common/testing/TestPaths.hh"

using namespace gz;
class ColladaLoader : public common::testing::AutoLogFixture { };

/////////////////////////////////////////////////
TEST_F(ColladaLoader, LoadZeroCount)
{
  common::ColladaLoader loader;
  std::unique_ptr<common::Mesh> mesh(loader.Load(
      common::testing::TestFile("data", "zero_count.dae")));
  ASSERT_TRUE(mesh);
#ifndef _WIN32
  common::Console::Root().RawLogger().flush();
  std::string log = LogContent();

  // Expect no errors about missing values
  EXPECT_EQ(log.find("Loading what we can..."), std::string::npos);
  EXPECT_EQ(log.find("Vertex source missing float_array"), std::string::npos);
  EXPECT_EQ(log.find("Normal source missing float_array"), std::string::npos);

  // Expect the logs to contain information
  EXPECT_NE(log.find("Triangle input has a count of zero"), std::string::npos);
  EXPECT_NE(log.find("Vertex source has a float_array with a count of zero"),
      std::string::npos);
  EXPECT_NE(log.find("Normal source has a float_array with a count of zero"),
      std::string::npos);
#endif
}

/////////////////////////////////////////////////
TEST_F(ColladaLoader, LoadBoxInstControllerWithoutSkeleton)
{
  common::ColladaLoader loader;
  std::unique_ptr<common::Mesh> mesh(loader.Load(
      common::testing::TestFile("data",
        "box_inst_controller_without_skeleton.dae")));

  EXPECT_EQ(36u, mesh->IndexCount());
  EXPECT_EQ(35u, mesh->VertexCount());
  EXPECT_EQ(1u, mesh->SubMeshCount());
  EXPECT_EQ(1u, mesh->MaterialCount());
  EXPECT_EQ(35u, mesh->TexCoordCount());
  common::SkeletonPtr skeleton = mesh->MeshSkeleton();
  EXPECT_LT(0u, skeleton->NodeCount());
  EXPECT_NE(nullptr, skeleton->NodeById("Armature_Bone"));
}

/////////////////////////////////////////////////
TEST_F(ColladaLoader, LoadBoxMultipleInstControllers)
{
  common::ColladaLoader loader;
  std::unique_ptr<common::Mesh> mesh(loader.Load(
      common::testing::TestFile("data", "box_multiple_inst_controllers.dae")));

  EXPECT_EQ(72u, mesh->IndexCount());
  EXPECT_EQ(70u, mesh->VertexCount());
  EXPECT_EQ(2u, mesh->SubMeshCount());
  EXPECT_EQ(1u, mesh->MaterialCount());
  EXPECT_EQ(70u, mesh->TexCoordCount());

  std::shared_ptr<common::SubMesh> submesh = mesh->SubMeshByIndex(0).lock();
  std::shared_ptr<common::SubMesh> submesh2 = mesh->SubMeshByIndex(1).lock();
  EXPECT_EQ(36u, submesh->IndexCount());
  EXPECT_EQ(36u, submesh2->IndexCount());
  EXPECT_EQ(35u, submesh->VertexCount());
  EXPECT_EQ(35u, submesh2->VertexCount());
  EXPECT_EQ(35u, submesh->TexCoordCount());
  EXPECT_EQ(35u, submesh2->TexCoordCount());

  common::SkeletonPtr skeleton = mesh->MeshSkeleton();
  EXPECT_NE(nullptr, skeleton->NodeById("Armature_Bone"));
  EXPECT_NE(nullptr, skeleton->NodeById("Armature_Bone2"));
}

/////////////////////////////////////////////////
TEST_F(ColladaLoader, LoadBoxNestedAnimation)
{
  common::ColladaLoader loader;
  std::unique_ptr<common::Mesh> mesh(loader.Load(
      common::testing::TestFile("data", "box_nested_animation.dae")));

  EXPECT_EQ(36u, mesh->IndexCount());
  EXPECT_EQ(35u, mesh->VertexCount());
  EXPECT_EQ(1u, mesh->SubMeshCount());
  EXPECT_EQ(1u, mesh->MaterialCount());
  EXPECT_EQ(35u, mesh->TexCoordCount());
  common::SkeletonPtr skeleton = mesh->MeshSkeleton();
  ASSERT_EQ(1u, mesh->MeshSkeleton()->AnimationCount());
  common::SkeletonAnimation *anim = skeleton->Animation(0);
  EXPECT_EQ(1u, anim->NodeCount());
  EXPECT_TRUE(anim->HasNode("Bone"));
  auto nodeAnimation = anim->NodeAnimationByName("Bone");
  EXPECT_NE(nullptr, nodeAnimation);
  EXPECT_EQ("Bone", nodeAnimation->Name());
  auto poseStart = anim->PoseAt(0);
  math::Matrix4d expectedTrans = math::Matrix4d(
      1, 0, 0, 1,
      0, 1, 0, -1,
      0, 0, 1, 0,
      0, 0, 0, 1);
  EXPECT_EQ(expectedTrans, poseStart.at("Bone"));
  auto poseEnd = anim->PoseAt(1.666667);
  expectedTrans = math::Matrix4d(
        1, 0, 0, 2,
        0, 1, 0, -1,
        0, 0, 1, 0,
        0, 0, 0, 1);
  EXPECT_EQ(expectedTrans, poseEnd.at("Bone"));
}

/////////////////////////////////////////////////
TEST_F(ColladaLoader, LoadBoxWithDefaultStride)
{
  common::ColladaLoader loader;
  std::unique_ptr<common::Mesh> mesh(loader.Load(
      common::testing::TestFile("data", "box_with_default_stride.dae")));

  EXPECT_EQ(36u, mesh->IndexCount());
  EXPECT_EQ(35u, mesh->VertexCount());
  EXPECT_EQ(1u, mesh->SubMeshCount());
  EXPECT_EQ(1u, mesh->MaterialCount());
  EXPECT_EQ(35u, mesh->TexCoordCount());
  ASSERT_EQ(1u, mesh->MeshSkeleton()->AnimationCount());
}

/////////////////////////////////////////////////
TEST_F(ColladaLoader, LoadBoxWithHierarchicalNodes)
{
  common::ColladaLoader loader;
  std::unique_ptr<common::Mesh> mesh(loader.Load(
      common::testing::TestFile("data", "box_with_hierarchical_nodes.dae")));

  ASSERT_EQ(5u, mesh->SubMeshCount());

  // node by itself
  EXPECT_EQ("StaticCube", mesh->SubMeshByIndex(0).lock()->Name());

  // nested node with no name so it takes the parent's name instead
  EXPECT_EQ("StaticCubeParent", mesh->SubMeshByIndex(1).lock()->Name());

  // parent node containing child node with no name
  EXPECT_EQ("StaticCubeParent", mesh->SubMeshByIndex(2).lock()->Name());

  // nested node with name
  EXPECT_EQ("StaticCubeNested", mesh->SubMeshByIndex(3).lock()->Name());

  // Parent of nested node with name
  EXPECT_EQ("StaticCubeParent2", mesh->SubMeshByIndex(4).lock()->Name());
}

/////////////////////////////////////////////////
TEST_F(ColladaLoader, MergeBoxWithDoubleSkeleton)
{
  common::ColladaLoader loader;
  std::unique_ptr<common::Mesh> mesh(loader.Load(
      common::testing::TestFile("data", "box_with_double_skeleton.dae")));
  EXPECT_TRUE(mesh->HasSkeleton());
  auto skeleton_ptr = mesh->MeshSkeleton();
  // The two skeletons have been joined and their root is the
  // animation root, called Armature
  EXPECT_EQ(skeleton_ptr->RootNode()->Name(), std::string("Armature"));
}

/////////////////////////////////////////////////
TEST_F(ColladaLoader, LoadCylinderAnimatedFrom3dsMax)
{
  // TODO(anyone) This test shows that the mesh loads without crashing, but the
  // mesh animation looks deformed when loaded. That still needs to be
  // addressed.
  common::ColladaLoader loader;
  std::unique_ptr<common::Mesh> mesh(loader.Load(
      common::testing::TestFile("data",
        "cylinder_animated_from_3ds_max.dae")));

  EXPECT_EQ("unknown", mesh->Name());
  EXPECT_EQ(202u, mesh->VertexCount());
  EXPECT_EQ(202u, mesh->NormalCount());
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
  EXPECT_EQ("Bone02", anim->Name());
  EXPECT_EQ(1u, anim->NodeCount());
  EXPECT_TRUE(anim->HasNode("Bone02"));
}

/////////////////////////////////////////////////
TEST_F(ColladaLoader, LoadLines)
{
  common::ColladaLoader loader;
  std::unique_ptr<common::Mesh> mesh(loader.Load(
      common::testing::TestFile("data", "xy_square_lines.dae")));
  ASSERT_TRUE(mesh);

  // 4 line segments, each contributes 2 vertices and 2 indices.
  EXPECT_EQ(1u, mesh->SubMeshCount());
  EXPECT_EQ(8u, mesh->VertexCount());
  EXPECT_EQ(8u, mesh->IndexCount());

  auto subMesh = mesh->SubMeshByIndex(0u).lock();
  ASSERT_NE(nullptr, subMesh);
  EXPECT_EQ(common::SubMesh::LINES, subMesh->SubMeshPrimitiveType());
}

/////////////////////////////////////////////////
TEST_F(ColladaLoader, LoadTextureMaterial)
{
  common::ColladaLoader loader;
  std::unique_ptr<common::Mesh> mesh(loader.Load(
      common::testing::TestFile("data", "xy_triangle_texture.dae")));
  ASSERT_TRUE(mesh);

  EXPECT_EQ(3u, mesh->VertexCount());
  EXPECT_EQ(3u, mesh->TexCoordCount());
  ASSERT_EQ(1u, mesh->MaterialCount());

  common::MaterialPtr mat = mesh->MaterialByIndex(0u);
  ASSERT_NE(nullptr, mat);

  // The diffuse texture chain should have been resolved and stored.
  EXPECT_FALSE(mat->TextureImage().empty());
  EXPECT_NE(mat->TextureImage().find("xy_triangle_texture.png"),
      std::string::npos);
}

/////////////////////////////////////////////////
// Loading a non-existent file must return null, not crash.
TEST_F(ColladaLoader, LoadNonexistentFile)
{
  common::ColladaLoader loader;
  std::unique_ptr<common::Mesh> mesh(loader.Load(
      common::testing::TestFile("data", "this_file_does_not_exist.dae")));
  EXPECT_EQ(nullptr, mesh);
}

/////////////////////////////////////////////////
// A file whose root element is not <COLLADA> must return null, not crash.
TEST_F(ColladaLoader, LoadNoColladaTag)
{
  common::ColladaLoader loader;
  std::unique_ptr<common::Mesh> mesh(loader.Load(
      common::testing::TestFile("data", "no_collada_tag.dae")));
  EXPECT_EQ(nullptr, mesh);
#ifndef _WIN32
  common::Console::Root().RawLogger().flush();
  EXPECT_NE(LogContent().find("Missing COLLADA tag"), std::string::npos);
#endif
}

/////////////////////////////////////////////////
// An unsupported version is reported but the mesh still loads.
TEST_F(ColladaLoader, LoadBadVersion)
{
  common::ColladaLoader loader;
  std::unique_ptr<common::Mesh> mesh(loader.Load(
      common::testing::TestFile("data", "bad_version.dae")));
  ASSERT_TRUE(mesh);
  EXPECT_EQ(3u, mesh->VertexCount());
#ifndef _WIN32
  common::Console::Root().RawLogger().flush();
  EXPECT_NE(LogContent().find("Invalid collada file"), std::string::npos);
#endif
}

/////////////////////////////////////////////////
// A <scene> referencing a missing visual_scene must be reported, not crash.
TEST_F(ColladaLoader, LoadMissingVisualScene)
{
  common::ColladaLoader loader;
  std::unique_ptr<common::Mesh> mesh(loader.Load(
      common::testing::TestFile("data", "missing_visual_scene.dae")));
  ASSERT_TRUE(mesh);
  EXPECT_EQ(0u, mesh->VertexCount());
#ifndef _WIN32
  common::Console::Root().RawLogger().flush();
  EXPECT_NE(LogContent().find("Unable to find visual_scene"),
      std::string::npos);
#endif
}

