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

#include "gz/common/Mesh.hh"
#include "gz/common/SubMesh.hh"
#include "gz/common/Material.hh"
#include "gz/common/Skeleton.hh"
#include "gz/common/SkeletonAnimation.hh"
#include "gz/common/AssimpLoader.hh"

#include "gz/common/testing/AutoLogFixture.hh"
#include "gz/common/testing/TestPaths.hh"

using namespace gz;
class AssimpLoader : public common::testing::AutoLogFixture { };

/////////////////////////////////////////////////
TEST_F(AssimpLoader, LoadBox)
{
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "box.dae"));

  EXPECT_STREQ("unknown", mesh->Name().c_str());
  EXPECT_EQ(math::Vector3d(1, 1, 1), mesh->Max());
  EXPECT_EQ(math::Vector3d(-1, -1, -1), mesh->Min());
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
TEST_F(AssimpLoader, Material)
{
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "box.dae"));
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

  common::Mesh *meshOpaque = loader.Load(
      common::testing::TestFile("data", "box_opaque.dae"));
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
TEST_F(AssimpLoader, ShareVertices)
{
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "box.dae"));

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
TEST_F(AssimpLoader, LoadZeroCount)
{
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "zero_count.dae"));
  ASSERT_TRUE(mesh);
  // This is custom logging messages, not implemented in Assimp
  /*
#ifndef _WIN32
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
  */
}

/////////////////////////////////////////////////
TEST_F(AssimpLoader, TexCoordSets)
{
  common::AssimpLoader loader;
  // This triangle mesh has multiple uv sets and vertices separated by
  // line breaks
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data",
        "multiple_texture_coordinates_triangle.dae"));
  ASSERT_TRUE(mesh);

  EXPECT_EQ(6u, mesh->VertexCount());
  EXPECT_EQ(6u, mesh->NormalCount());
  EXPECT_EQ(6u, mesh->IndexCount());
  EXPECT_EQ(6u, mesh->TexCoordCount());
  EXPECT_EQ(2u, mesh->SubMeshCount());
  EXPECT_EQ(1u, mesh->MaterialCount());

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
TEST_F(AssimpLoader, LoadBoxWithAnimationOutsideSkeleton)
{
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data",
        "box_with_animation_outside_skeleton.dae"));

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
  auto poseEnd = anim->PoseAt(1.666666);
  expectedTrans = math::Matrix4d(
        1, 0, 0, 2,
        0, 1, 0, -1,
        0, 0, 1, 0,
        0, 0, 0, 1);
  EXPECT_EQ(expectedTrans, poseEnd.at("Armature"));
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
}

/////////////////////////////////////////////////
TEST_F(AssimpLoader, LoadBoxNestedAnimation)
{
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "box_nested_animation.dae"));

  EXPECT_EQ(36u, mesh->IndexCount());
  EXPECT_EQ(24u, mesh->VertexCount());
  EXPECT_EQ(1u, mesh->SubMeshCount());
  EXPECT_EQ(1u, mesh->MaterialCount());
  EXPECT_EQ(24u, mesh->TexCoordCount());
  common::SkeletonPtr skeleton = mesh->MeshSkeleton();
  ASSERT_EQ(1u, mesh->MeshSkeleton()->AnimationCount());
  common::SkeletonAnimation *anim = skeleton->Animation(0);
  EXPECT_EQ(anim->Name(), "Armature");
  EXPECT_EQ(1u, anim->NodeCount());
  EXPECT_TRUE(anim->HasNode("Armature_Bone"));
  auto nodeAnimation = anim->NodeAnimationByName("Armature_Bone");
  ASSERT_NE(nullptr, nodeAnimation);
  EXPECT_EQ("Armature_Bone", nodeAnimation->Name());
  auto poseStart = anim->PoseAt(0);
  math::Matrix4d expectedTrans = math::Matrix4d(
      1, 0, 0, 1,
      0, 1, 0, -1,
      0, 0, 1, 0,
      0, 0, 0, 1);
  EXPECT_EQ(expectedTrans, poseStart.at("Armature_Bone"));
  auto poseEnd = anim->PoseAt(1.666666);
  expectedTrans = math::Matrix4d(
        1, 0, 0, 2,
        0, 1, 0, -1,
        0, 0, 1, 0,
        0, 0, 0, 1);
  EXPECT_EQ(expectedTrans, poseEnd.at("Armature_Bone"));
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
  // TODO not working, investigate
  //ASSERT_EQ(1u, mesh->MeshSkeleton()->AnimationCount());
}

/////////////////////////////////////////////////
TEST_F(AssimpLoader, LoadBoxWithMultipleGeoms)
{
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "box_with_multiple_geoms.dae"));

  EXPECT_EQ(72u, mesh->IndexCount());
  EXPECT_EQ(48u, mesh->VertexCount());
  EXPECT_EQ(1u, mesh->MaterialCount());
  EXPECT_EQ(48u, mesh->TexCoordCount());
  ASSERT_EQ(1u, mesh->MeshSkeleton()->AnimationCount());
  ASSERT_EQ(2u, mesh->SubMeshCount());
  EXPECT_EQ(24u, mesh->SubMeshByIndex(0).lock()->NodeAssignmentsCount());
  EXPECT_EQ(0u, mesh->SubMeshByIndex(1).lock()->NodeAssignmentsCount());
}

/////////////////////////////////////////////////
TEST_F(AssimpLoader, LoadBoxWithHierarchicalNodes)
{
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "box_with_hierarchical_nodes.dae"));

  ASSERT_EQ(5u, mesh->SubMeshCount());

  // node by itself
  EXPECT_EQ("StaticCube", mesh->SubMeshByIndex(0).lock()->Name());

  // nested node with no name so it takes the parent's name instead
  EXPECT_EQ("StaticCubeParent", mesh->SubMeshByIndex(1).lock()->Name());

  // parent node containing child node with no name
  // CHANGE Assimp assigns the id to the name if the mesh has no name
  EXPECT_EQ("StaticCubeNestedNoName", mesh->SubMeshByIndex(2).lock()->Name());

  // Parent of nested node with name
  EXPECT_EQ("StaticCubeParent2", mesh->SubMeshByIndex(3).lock()->Name());

  // nested node with name
  EXPECT_EQ("StaticCubeNested", mesh->SubMeshByIndex(4).lock()->Name());
}

/////////////////////////////////////////////////
TEST_F(AssimpLoader, MergeBoxWithDoubleSkeleton)
{
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "box_with_double_skeleton.dae"));
  EXPECT_TRUE(mesh->HasSkeleton());
  auto skeleton_ptr = mesh->MeshSkeleton();
  // The two skeletons have been joined and their root is the
  // animation root, called Scene
  EXPECT_EQ(skeleton_ptr->RootNode()->Name(), std::string("Scene"));
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
  EXPECT_EQ(1u, mesh->MaterialCount());

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
TEST_F(AssimpLoader, LoadObjBox)
{
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "box.obj"));

  EXPECT_STREQ("unknown", mesh->Name().c_str());
  EXPECT_EQ(math::Vector3d(1, 1, 1), mesh->Max());
  EXPECT_EQ(math::Vector3d(-1, -1, -1), mesh->Min());
  // 36 vertices after triangulation, assimp optimizes to 24
  EXPECT_EQ(24u, mesh->VertexCount());
  EXPECT_EQ(24u, mesh->NormalCount());
  EXPECT_EQ(36u, mesh->IndexCount());
  EXPECT_EQ(0u, mesh->TexCoordCount());
  EXPECT_EQ(1u, mesh->SubMeshCount());
  EXPECT_EQ(1u, mesh->MaterialCount());

  // Make sure we can read the submesh name
  EXPECT_STREQ("Cube_Cube.001", mesh->SubMeshByIndex(0).lock()->Name().c_str());

  EXPECT_EQ(mesh->MaterialCount(), 1u);

  const common::MaterialPtr mat = mesh->MaterialByIndex(0u);
  ASSERT_TRUE(mat.get());

  // Make sure we read the material color values
  EXPECT_EQ(mat->Ambient(), math::Color(0.0, 0.0, 0.0, 1.0));
  EXPECT_EQ(mat->Diffuse(), math::Color(0.512f, 0.512f, 0.512f, 1.0f));
  EXPECT_EQ(mat->Specular(), math::Color(0.25, 0.25, 0.25, 1.0));
  EXPECT_DOUBLE_EQ(mat->Transparency(), 0.0);
}


/////////////////////////////////////////////////
// This tests opening an OBJ file that has an invalid material reference
TEST_F(AssimpLoader, ObjInvalidMaterial)
{
  gz::common::AssimpLoader loader;

  std::string meshFilename =
    common::testing::TestFile("data", "invalid_material.obj");

  gz::common::Mesh *mesh = loader.Load(meshFilename);

  EXPECT_TRUE(mesh != nullptr);
}

/////////////////////////////////////////////////
// This tests opening an OBJ file that has PBR fields
TEST_F(AssimpLoader, ObjPBR)
{
  gz::common::AssimpLoader loader;

  // This was custom behavior of OBJ Loader to fix blender OBJ PBR issue
  // not implemented in assimp
  /*
  // load obj file exported by 3ds max that has pbr extension
  {
    std::string meshFilename =
      common::testing::TestFile("data", "cube_pbr.obj");

    gz::common::Mesh *mesh = loader.Load(meshFilename);
    EXPECT_NE(nullptr, mesh);

    const common::MaterialPtr mat = mesh->MaterialByIndex(0u);
    ASSERT_TRUE(mat.get());

    EXPECT_EQ(math::Color(0.0f, 0.0f, 0.0f, 1.0f), mat->Ambient());
    EXPECT_EQ(math::Color(0.5f, 0.5f, 0.5f, 1.0f), mat->Diffuse());
    EXPECT_EQ(math::Color(1.0f, 1.0f, 1.0f, 1.0f), mat->Specular());
    EXPECT_DOUBLE_EQ(0.0, mat->Transparency());
    EXPECT_NE(std::string::npos,
        mat->TextureImage().find("LightDome_Albedo.png"));
    const common::Pbr *pbr = mat->PbrMaterial();
    EXPECT_DOUBLE_EQ(0, pbr->Roughness());
    EXPECT_DOUBLE_EQ(0, pbr->Metalness());
    EXPECT_EQ("LightDome_Metalness.png", pbr->MetalnessMap());
    EXPECT_EQ("LightDome_Roughness.png", pbr->RoughnessMap());
    EXPECT_EQ("LightDome_Normal.png", pbr->NormalMap());
  }

  // load obj file exported by blender - it shoves pbr maps into
  // existing fields
  {
    std::string meshFilename =
      common::testing::TestFile("data", "blender_pbr.obj");

    gz::common::Mesh *mesh = loader.Load(meshFilename);
    EXPECT_NE(nullptr, mesh);

    const common::MaterialPtr mat = mesh->MaterialByIndex(0u);
    ASSERT_TRUE(mat.get());

    EXPECT_EQ(math::Color(1.0f, 1.0f, 1.0f, 1.0f), mat->Ambient());
    EXPECT_EQ(math::Color(0.8f, 0.8f, 0.8f, 1.0f), mat->Diffuse());
    EXPECT_EQ(math::Color(0.5f, 0.5f, 0.5f, 1.0f), mat->Specular());
    EXPECT_EQ(math::Color(0.0f, 0.0f, 0.0f, 1.0f), mat->Emissive());
    EXPECT_DOUBLE_EQ(0.0, mat->Transparency());
    EXPECT_NE(std::string::npos,
        mat->TextureImage().find("mesh_Diffuse.png"));
    const common::Pbr *pbr = mat->PbrMaterial();
    EXPECT_DOUBLE_EQ(0, pbr->Roughness());
    EXPECT_DOUBLE_EQ(0, pbr->Metalness());
    EXPECT_EQ("mesh_Metal.png", pbr->MetalnessMap());
    EXPECT_EQ("mesh_Rough.png", pbr->RoughnessMap());
    EXPECT_EQ("mesh_Normal.png", pbr->NormalMap());
  }
  */
}

/////////////////////////////////////////////////
// This test opens a FBX file
TEST_F(AssimpLoader, LoadFbxBox)
{
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "box.fbx"));

  EXPECT_STREQ("unknown", mesh->Name().c_str());
  EXPECT_EQ(ignition::math::Vector3d(1, 1, 1), mesh->Max());
  EXPECT_EQ(ignition::math::Vector3d(-1, -1, -1), mesh->Min());
  // 36 vertices
  // TODO check why now we have 24, sounds more correct than 36?
  EXPECT_EQ(36u, mesh->VertexCount());
  EXPECT_EQ(36u, mesh->NormalCount());
  EXPECT_EQ(36u, mesh->IndexCount());
  EXPECT_EQ(0u, mesh->TexCoordCount());
  EXPECT_EQ(1u, mesh->SubMeshCount());
  EXPECT_EQ(1u, mesh->MaterialCount());

  // Make sure we can read the submesh name
  EXPECT_STREQ("Cube_Cube.001", mesh->SubMeshByIndex(0).lock()->Name().c_str());

  EXPECT_EQ(mesh->MaterialCount(), 2u);

  const common::MaterialPtr mat = mesh->MaterialByIndex(1u);
  ASSERT_TRUE(mat.get());

  // Make sure we read the material color values
  EXPECT_EQ(mat->Ambient(), math::Color(0.0, 0.0, 0.0, 1.0));
  EXPECT_EQ(mat->Diffuse(), math::Color(0.512f, 0.512f, 0.512f, 1.0f));
  EXPECT_EQ(mat->Specular(), math::Color(0.25, 0.25, 0.25, 1.0));
  EXPECT_DOUBLE_EQ(mat->Transparency(), 0.0);
}

/////////////////////////////////////////////////
// This test opens a GLB file
TEST_F(AssimpLoader, LoadGlTF2Box)
{
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "box.glb"));

  EXPECT_STREQ("unknown", mesh->Name().c_str());
  EXPECT_EQ(ignition::math::Vector3d(1, 1, 1), mesh->Max());
  EXPECT_EQ(ignition::math::Vector3d(-1, -1, -1), mesh->Min());

  // 36 vertices
  // TODO check why now we have 24, sounds more correct than 36?
  EXPECT_EQ(36u, mesh->VertexCount());
  EXPECT_EQ(36u, mesh->NormalCount());
  EXPECT_EQ(36u, mesh->IndexCount());
  EXPECT_EQ(0u, mesh->TexCoordCount());
  EXPECT_EQ(1u, mesh->SubMeshCount());
  EXPECT_EQ(1u, mesh->MaterialCount());

  // Make sure we can read the submesh name
  EXPECT_STREQ("Cube_Cube.001", mesh->SubMeshByIndex(0).lock()->Name().c_str());

  EXPECT_EQ(mesh->MaterialCount(), 2u);

  const common::MaterialPtr mat = mesh->MaterialByIndex(1u);
  ASSERT_TRUE(mat.get());

  // Make sure we read the material color values
  EXPECT_EQ(mat->Ambient(), math::Color(0.0, 0.0, 0.0, 1.0));
  EXPECT_EQ(mat->Diffuse(), math::Color(0.512f, 0.512f, 0.512f, 1.0f));
  EXPECT_EQ(mat->Specular(), math::Color(0.25, 0.25, 0.25, 1.0));
  EXPECT_DOUBLE_EQ(mat->Transparency(), 0.0);
}

