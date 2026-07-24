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
TEST_F(ColladaLoader, LoadBox)
{
  common::ColladaLoader loader;
  std::unique_ptr<common::Mesh> mesh(loader.Load(
      common::testing::TestFile("data", "box.dae")));

  EXPECT_STREQ("unknown", mesh->Name().c_str());
  EXPECT_EQ(gz::math::Vector3d(1, 1, 1), mesh->Max());
  EXPECT_EQ(gz::math::Vector3d(-1, -1, -1), mesh->Min());
  // 36 vertices, 24 unique, 12 shared.
  EXPECT_EQ(24u, mesh->VertexCount());
  EXPECT_EQ(24u, mesh->NormalCount());
  EXPECT_EQ(36u, mesh->IndexCount());
  EXPECT_EQ(0u, mesh->TexCoordCount());
  EXPECT_EQ(1u, mesh->SubMeshCount());
  EXPECT_EQ(1u, mesh->MaterialCount());

  // Make sure we can read a submesh name
  EXPECT_STREQ("Cube", mesh->SubMeshByIndex(0).lock()->Name().c_str());
}

/////////////////////////////////////////////////
TEST_F(ColladaLoader, ShareVertices)
{
  common::ColladaLoader loader;
  std::unique_ptr<common::Mesh> mesh(loader.Load(
      common::testing::TestFile("data", "box.dae")));

  // check number of shared vertices
  std::set<unsigned int> uniqueIndices;
  int shared = 0;
  for (unsigned int i = 0; i < mesh->SubMeshCount(); ++i)
  {
    const std::shared_ptr<common::SubMesh> subMesh =
      mesh->SubMeshByIndex(i).lock();
    for (unsigned int j = 0; j < subMesh->IndexCount(); ++j)
    {
      if (uniqueIndices.find(subMesh->Index(j)) == uniqueIndices.end())
      {
        uniqueIndices.insert(subMesh->Index(j));
      }
      else
      {
        shared++;
      }
    }
  }
  EXPECT_EQ(shared, 12);
  EXPECT_EQ(uniqueIndices.size(), 24u);

  // check all vertices are unique
  for (unsigned int i = 0; i < mesh->SubMeshCount(); ++i)
  {
    const std::shared_ptr<common::SubMesh> subMesh =
      mesh->SubMeshByIndex(i).lock();
    for (unsigned int j = 0; j < subMesh->VertexCount(); ++j)
    {
      gz::math::Vector3d v = subMesh->Vertex(j);
      gz::math::Vector3d n = subMesh->Normal(j);

      // Verify there is no other vertex with the same position AND normal
      for (unsigned int k = j+1; k < subMesh->VertexCount(); ++k)
      {
        if (v == subMesh->Vertex(k))
        {
          EXPECT_TRUE(n != subMesh->Normal(k));
        }
      }
    }
  }
}

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
TEST_F(ColladaLoader, Material)
{
  common::ColladaLoader loader;
  std::unique_ptr<common::Mesh> mesh(loader.Load(
      common::testing::TestFile("data", "box.dae")));
  ASSERT_TRUE(mesh);

  EXPECT_EQ(mesh->MaterialCount(), 1u);

  common::MaterialPtr mat = mesh->MaterialByIndex(0u);
  ASSERT_TRUE(mat != nullptr);

  // Make sure we read the specular value
  EXPECT_EQ(math::Color(0.0, 0.0, 0.0, 1.0), mat->Ambient());
  EXPECT_EQ(math::Color(0.64f, 0.64f, 0.64f, 1.0f), mat->Diffuse());
  EXPECT_EQ(math::Color(0.5, 0.5, 0.5, 1.0), mat->Specular());
  EXPECT_EQ(math::Color(0.0, 0.0, 0.0, 1.0), mat->Emissive());
  EXPECT_DOUBLE_EQ(50.0, mat->Shininess());
  // transparent: opaque="A_ONE", color=[1 1 1 1]
  // transparency: 1.0
  // resulting transparency value = (1 - color.a * transparency)
  EXPECT_DOUBLE_EQ(0.0, mat->Transparency());
  double srcFactor = -1;
  double dstFactor = -1;
  mat->BlendFactors(srcFactor, dstFactor);
  EXPECT_DOUBLE_EQ(1.0, srcFactor);
  EXPECT_DOUBLE_EQ(0.0, dstFactor);

  std::unique_ptr<common::Mesh> meshOpaque(loader.Load(
      common::testing::TestFile("data", "box_opaque.dae")));
  ASSERT_TRUE(meshOpaque);

  EXPECT_EQ(meshOpaque->MaterialCount(), 1u);

  common::MaterialPtr matOpaque = meshOpaque->MaterialByIndex(0u);
  ASSERT_TRUE(matOpaque != nullptr);

  // Make sure we read the specular value
  EXPECT_EQ(math::Color(0.0, 0.0, 0.0, 1.0), matOpaque->Ambient());
  EXPECT_EQ(math::Color(0.64f, 0.64f, 0.64f, 1.0f), matOpaque->Diffuse());
  EXPECT_EQ(math::Color(0.5, 0.5, 0.5, 1.0), matOpaque->Specular());
  EXPECT_EQ(math::Color(0.0, 0.0, 0.0, 1.0), matOpaque->Emissive());
  EXPECT_DOUBLE_EQ(50.0, matOpaque->Shininess());
  // transparent: opaque="A_ONE", color=[1 1 1 1]
  // transparency: not specified, defaults to 1.0
  // resulting transparency value = (1 - color.a * transparency)
  EXPECT_DOUBLE_EQ(0.0, matOpaque->Transparency());
  srcFactor = -1;
  dstFactor = -1;
  matOpaque->BlendFactors(srcFactor, dstFactor);
  EXPECT_DOUBLE_EQ(1.0, srcFactor);
  EXPECT_DOUBLE_EQ(0.0, dstFactor);

}

/////////////////////////////////////////////////
TEST_F(ColladaLoader, TexCoordSets)
{
  common::ColladaLoader loader;
  // This triangle mesh has multiple uv sets and vertices separated by
  // line breaks
  std::unique_ptr<common::Mesh> mesh(loader.Load(
      common::testing::TestFile("data",
        "multiple_texture_coordinates_triangle.dae")));
  ASSERT_TRUE(mesh);

  EXPECT_EQ(6u, mesh->VertexCount());
  EXPECT_EQ(6u, mesh->NormalCount());
  EXPECT_EQ(6u, mesh->IndexCount());
  EXPECT_EQ(6u, mesh->TexCoordCount());
  EXPECT_EQ(2u, mesh->SubMeshCount());
  EXPECT_EQ(0u, mesh->MaterialCount());

  auto sm = mesh->SubMeshByIndex(0u);
  auto subMesh = sm.lock();
  EXPECT_NE(nullptr, subMesh);
  EXPECT_EQ(math::Vector3d(0, 0, 0), subMesh->Vertex(0u));
  EXPECT_EQ(math::Vector3d(10, 0, 0), subMesh->Vertex(1u));
  EXPECT_EQ(math::Vector3d(10, 10, 0), subMesh->Vertex(2u));
  EXPECT_EQ(math::Vector3d(0, 0, 1), subMesh->Normal(0u));
  EXPECT_EQ(math::Vector3d(0, 0, 1), subMesh->Normal(1u));
  EXPECT_EQ(math::Vector3d(0, 0, 1), subMesh->Normal(2u));
  EXPECT_EQ(math::Vector2d(0, 1), subMesh->TexCoord(0u));
  EXPECT_EQ(math::Vector2d(0, 1), subMesh->TexCoord(1u));
  EXPECT_EQ(math::Vector2d(0, 1), subMesh->TexCoord(2u));

  auto smb = mesh->SubMeshByIndex(1u);
  auto subMeshB = smb.lock();
  EXPECT_NE(nullptr, subMeshB);
  EXPECT_EQ(math::Vector3d(10, 0, 0), subMeshB->Vertex(0u));
  EXPECT_EQ(math::Vector3d(20, 0, 0), subMeshB->Vertex(1u));
  EXPECT_EQ(math::Vector3d(20, 10, 0), subMeshB->Vertex(2u));
  EXPECT_EQ(math::Vector3d(0, 0, 1), subMeshB->Normal(0u));
  EXPECT_EQ(math::Vector3d(0, 0, 1), subMeshB->Normal(1u));
  EXPECT_EQ(math::Vector3d(0, 0, 1), subMeshB->Normal(2u));
  EXPECT_EQ(math::Vector2d(0, 1), subMeshB->TexCoord(0u));
  EXPECT_EQ(math::Vector2d(0, 1), subMeshB->TexCoord(1u));
  EXPECT_EQ(math::Vector2d(0, 1), subMeshB->TexCoord(2u));

  EXPECT_TRUE(subMeshB->HasTexCoord(0u));
  EXPECT_TRUE(subMeshB->HasTexCoord(1u));
  EXPECT_TRUE(subMeshB->HasTexCoord(2u));
  EXPECT_FALSE(subMeshB->HasTexCoord(3u));

  // test texture coordinate set API
  EXPECT_EQ(3u, subMeshB->TexCoordSetCount());
  EXPECT_EQ(3u, subMeshB->TexCoordCountBySet(0u));
  EXPECT_EQ(math::Vector2d(0, 1), subMeshB->TexCoordBySet(0u, 0u));
  EXPECT_EQ(math::Vector2d(0, 1), subMeshB->TexCoordBySet(1u, 0u));
  EXPECT_EQ(math::Vector2d(0, 1), subMeshB->TexCoordBySet(2u, 0u));

  EXPECT_TRUE(subMeshB->HasTexCoordBySet(0u, 0u));
  EXPECT_TRUE(subMeshB->HasTexCoordBySet(1u, 0u));
  EXPECT_TRUE(subMeshB->HasTexCoordBySet(2u, 0u));
  EXPECT_FALSE(subMeshB->HasTexCoordBySet(3u, 0u));

  EXPECT_EQ(3u, subMeshB->TexCoordCountBySet(1u));
  EXPECT_EQ(math::Vector2d(0, 0.5), subMeshB->TexCoordBySet(0u, 1u));
  EXPECT_EQ(math::Vector2d(0, 0.4), subMeshB->TexCoordBySet(1u, 1u));
  EXPECT_EQ(math::Vector2d(0, 0.3), subMeshB->TexCoordBySet(2u, 1u));

  EXPECT_TRUE(subMeshB->HasTexCoordBySet(0u, 1u));
  EXPECT_TRUE(subMeshB->HasTexCoordBySet(1u, 1u));
  EXPECT_TRUE(subMeshB->HasTexCoordBySet(2u, 1u));
  EXPECT_FALSE(subMeshB->HasTexCoordBySet(3u, 1u));

  EXPECT_EQ(3u, subMeshB->TexCoordCountBySet(2u));
  EXPECT_EQ(math::Vector2d(0, 0.8), subMeshB->TexCoordBySet(0u, 2u));
  EXPECT_EQ(math::Vector2d(0, 0.7), subMeshB->TexCoordBySet(1u, 2u));
  EXPECT_EQ(math::Vector2d(0, 0.6), subMeshB->TexCoordBySet(2u, 2u));

  EXPECT_TRUE(subMeshB->HasTexCoordBySet(0u, 2u));
  EXPECT_TRUE(subMeshB->HasTexCoordBySet(1u, 2u));
  EXPECT_TRUE(subMeshB->HasTexCoordBySet(2u, 2u));
  EXPECT_FALSE(subMeshB->HasTexCoordBySet(3u, 2u));

  subMeshB->SetTexCoordBySet(2u, math::Vector2d(0.1, 0.2), 1u);
  EXPECT_EQ(math::Vector2d(0.1, 0.2), subMeshB->TexCoordBySet(2u, 1u));
}

/////////////////////////////////////////////////
TEST_F(ColladaLoader, LoadBoxWithAnimationOutsideSkeleton)
{
  common::ColladaLoader loader;
  std::unique_ptr<common::Mesh> mesh(loader.Load(
      common::testing::TestFile("data",
        "box_with_animation_outside_skeleton.dae")));

  EXPECT_EQ(36u, mesh->IndexCount());
  EXPECT_EQ(1u, mesh->SubMeshCount());
  EXPECT_EQ(1u, mesh->MaterialCount());
  EXPECT_LT(0u, mesh->TexCoordCount());
  common::SkeletonPtr skeleton = mesh->MeshSkeleton();
  ASSERT_EQ(1u, skeleton->AnimationCount());
  common::SkeletonAnimation *anim = skeleton->Animation(0);
  EXPECT_EQ(1u, anim->NodeCount());
  EXPECT_TRUE(anim->HasNode("Armature"));
  auto nodeAnimation = anim->NodeAnimationByName("Armature");
  EXPECT_NE(nullptr, nodeAnimation);
  EXPECT_EQ("Armature", nodeAnimation->Name());
  auto poseStart = anim->PoseAt(0.04166662);
  math::Matrix4d expectedTrans = math::Matrix4d(
      1, 0, 0, 1,
      0, 1, 0, -1,
      0, 0, 1, 0,
      0, 0, 0, 1);
  EXPECT_EQ(expectedTrans, poseStart.at("Armature"));
  auto poseEnd = anim->PoseAt(1.666667);
  expectedTrans = math::Matrix4d(
        1, 0, 0, 2,
        0, 1, 0, -1,
        0, 0, 1, 0,
        0, 0, 0, 1);
  EXPECT_EQ(expectedTrans, poseEnd.at("Armature"));
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
TEST_F(ColladaLoader, LoadBoxWithMultipleGeoms)
{
  common::ColladaLoader loader;
  std::unique_ptr<common::Mesh> mesh(loader.Load(
      common::testing::TestFile("data", "box_with_multiple_geoms.dae")));

  EXPECT_EQ(72u, mesh->IndexCount());
  EXPECT_EQ(48u, mesh->VertexCount());
  EXPECT_EQ(0u, mesh->MaterialCount());
  EXPECT_EQ(48u, mesh->TexCoordCount());
  ASSERT_EQ(1u, mesh->MeshSkeleton()->AnimationCount());
  ASSERT_EQ(2u, mesh->SubMeshCount());
  EXPECT_EQ(24u, mesh->SubMeshByIndex(0).lock()->NodeAssignmentsCount());
  EXPECT_EQ(0u, mesh->SubMeshByIndex(1).lock()->NodeAssignmentsCount());
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
// Load animation without a name
TEST_F(ColladaLoader, NoAnimName)
{
  common::ColladaLoader loader;

  std::string meshFilename =
    common::testing::TestFile("data", "box_with_no_animation_name.dae");

  std::unique_ptr<common::Mesh> mesh(loader.Load(meshFilename));
  common::SkeletonPtr skeleton = mesh->MeshSkeleton();
  ASSERT_EQ(1u, skeleton->AnimationCount());
  common::SkeletonAnimation *anim = skeleton->Animation(0);
  auto animName = anim->Name();
  EXPECT_EQ(animName, "animation1");
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
  EXPECT_NE(mat->TextureImage().find("xy_triangle_texture.png"), std::string::npos);
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


/////////////////////////////////////////////////
// A malformed <float_array> (missing count attribute) must be reported and
// must not crash the loader.
TEST_F(ColladaLoader, LoadMalformedPositionNoCount)
{
  common::ColladaLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "malformed_position_no_count.dae"));
  ASSERT_TRUE(mesh);
#ifndef _WIN32
  common::Console::Root().RawLogger().flush();
  EXPECT_NE(LogContent().find("no count attribute in position"),
      std::string::npos);
#endif
  delete mesh;
}

/////////////////////////////////////////////////
// A non-numeric count attribute must be caught (not throw) and reported.
TEST_F(ColladaLoader, LoadMalformedPositionBadCount)
{
  common::ColladaLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "malformed_position_bad_count.dae"));
  ASSERT_TRUE(mesh);
#ifndef _WIN32
  common::Console::Root().RawLogger().flush();
  EXPECT_NE(LogContent().find("Invalid count attribute in position"),
      std::string::npos);
#endif
  delete mesh;
}

/////////////////////////////////////////////////
// A missing accessor stride attribute must be reported and must not crash.
TEST_F(ColladaLoader, LoadMalformedPositionNoStride)
{
  common::ColladaLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "malformed_position_no_stride.dae"));
  ASSERT_TRUE(mesh);
#ifndef _WIN32
  common::Console::Root().RawLogger().flush();
  EXPECT_NE(LogContent().find("no stride attribute in position"),
      std::string::npos);
#endif
  delete mesh;
}

/////////////////////////////////////////////////
// An overflowing float value must be handled gracefully (not throw).
TEST_F(ColladaLoader, LoadMalformedPositionOverflow)
{
  common::ColladaLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "malformed_position_overflow.dae"));
  ASSERT_TRUE(mesh);
#ifndef _WIN32
  common::Console::Root().RawLogger().flush();
  EXPECT_NE(LogContent().find("Overflow while parsing <float_array>"),
      std::string::npos);
#endif
  delete mesh;
}

/////////////////////////////////////////////////
TEST_F(ColladaLoader, LoadMalformedNormalNoCount)
{
  common::ColladaLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "malformed_normal_no_count.dae"));
  ASSERT_TRUE(mesh);
#ifndef _WIN32
  common::Console::Root().RawLogger().flush();
  EXPECT_NE(LogContent().find("no count attribute in normal"),
      std::string::npos);
#endif
  delete mesh;
}

/////////////////////////////////////////////////
TEST_F(ColladaLoader, LoadMalformedNormalBadCount)
{
  common::ColladaLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "malformed_normal_bad_count.dae"));
  ASSERT_TRUE(mesh);
#ifndef _WIN32
  common::Console::Root().RawLogger().flush();
  EXPECT_NE(LogContent().find("Invalid count attribute in normal"),
      std::string::npos);
#endif
  delete mesh;
}

/////////////////////////////////////////////////
TEST_F(ColladaLoader, LoadMalformedNormalNoStride)
{
  common::ColladaLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "malformed_normal_no_stride.dae"));
  ASSERT_TRUE(mesh);
#ifndef _WIN32
  common::Console::Root().RawLogger().flush();
  EXPECT_NE(LogContent().find("no stride attribute in normal"),
      std::string::npos);
#endif
  delete mesh;
}

/////////////////////////////////////////////////
TEST_F(ColladaLoader, LoadMalformedTexcoordNoCount)
{
  common::ColladaLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "malformed_texcoord_no_count.dae"));
  ASSERT_TRUE(mesh);
#ifndef _WIN32
  common::Console::Root().RawLogger().flush();
  EXPECT_NE(LogContent().find("no count attribute in texture coordinate"),
      std::string::npos);
#endif
  delete mesh;
}

/////////////////////////////////////////////////
TEST_F(ColladaLoader, LoadMalformedTexcoordBadCount)
{
  common::ColladaLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "malformed_texcoord_bad_count.dae"));
  ASSERT_TRUE(mesh);
#ifndef _WIN32
  common::Console::Root().RawLogger().flush();
  EXPECT_NE(LogContent().find("Invalid count attribute in texture coordinate"),
      std::string::npos);
#endif
  delete mesh;
}

/////////////////////////////////////////////////
TEST_F(ColladaLoader, LoadMalformedTexcoordNoStride)
{
  common::ColladaLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "malformed_texcoord_no_stride.dae"));
  ASSERT_TRUE(mesh);
#ifndef _WIN32
  common::Console::Root().RawLogger().flush();
  EXPECT_NE(LogContent().find("no stride attribute in texture coordinate"),
      std::string::npos);
#endif
  delete mesh;
}

/////////////////////////////////////////////////
// A texcoord source whose accessor count*stride disagrees with the
// float_array count must be reported, not crash.
TEST_F(ColladaLoader, LoadMalformedTexcoordMismatch)
{
  common::ColladaLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "malformed_texcoord_mismatch.dae"));
  ASSERT_TRUE(mesh);
#ifndef _WIN32
  common::Console::Root().RawLogger().flush();
  EXPECT_NE(LogContent().find("Error reading texture coordinates"),
      std::string::npos);
#endif
  delete mesh;
}


/////////////////////////////////////////////////
// A zero accessor stride must be rejected: it previously caused an
// infinite read loop (and a division by zero in later revisions).
TEST_F(ColladaLoader, LoadMalformedPositionZeroStride)
{
  common::ColladaLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "malformed_position_zero_stride.dae"));
  ASSERT_TRUE(mesh);
  EXPECT_EQ(0u, mesh->VertexCount());
#ifndef _WIN32
  common::Console::Root().RawLogger().flush();
  EXPECT_NE(LogContent().find("Invalid stride attribute in position"),
      std::string::npos);
#endif
  delete mesh;
}

/////////////////////////////////////////////////
// A negative float_array count must be rejected: it previously drove a
// huge (wrapped-around) allocation that threw std::length_error.
TEST_F(ColladaLoader, LoadMalformedPositionNegativeCount)
{
  common::ColladaLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data",
      "malformed_position_negative_count.dae"));
  ASSERT_TRUE(mesh);
  EXPECT_EQ(0u, mesh->VertexCount());
#ifndef _WIN32
  common::Console::Root().RawLogger().flush();
  EXPECT_NE(LogContent().find("Invalid count attribute in position"),
      std::string::npos);
#endif
  delete mesh;
}

/////////////////////////////////////////////////
// A count far larger than the actual data must neither over-allocate nor
// read out of bounds; the values that are present are loaded.
TEST_F(ColladaLoader, LoadMalformedPositionHugeCount)
{
  common::ColladaLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "malformed_position_huge_count.dae"));
  ASSERT_TRUE(mesh);
  EXPECT_EQ(3u, mesh->VertexCount());
  delete mesh;
}

/////////////////////////////////////////////////
// An empty <init_from/> element must be reported, not crash.
TEST_F(ColladaLoader, LoadEmptyInitFrom)
{
  common::ColladaLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "empty_init_from.dae"));
  ASSERT_TRUE(mesh);
  EXPECT_EQ(3u, mesh->VertexCount());
#ifndef _WIN32
  common::Console::Root().RawLogger().flush();
  EXPECT_NE(LogContent().find("Empty <init_from> element"),
      std::string::npos);
#endif
  delete mesh;
}
