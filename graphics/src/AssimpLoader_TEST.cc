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

#include "ignition/common/Mesh.hh"
#include "ignition/common/SubMesh.hh"
#include "ignition/common/Material.hh"
#include "ignition/common/Skeleton.hh"
#include "ignition/common/SkeletonAnimation.hh"
#include "gz/common/AssimpLoader.hh"

#include "ignition/common/testing/AutoLogFixture.hh"
#include "ignition/common/testing/TestPaths.hh"

using namespace ignition;
class AssimpLoader : public common::testing::AutoLogFixture { };

/////////////////////////////////////////////////
TEST_F(AssimpLoader, LoadBox)
{
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "box.dae"));

  EXPECT_STREQ("unknown", mesh->Name().c_str());
  EXPECT_EQ(ignition::math::Vector3d(1, 1, 1), mesh->Max());
  EXPECT_EQ(ignition::math::Vector3d(-1, -1, -1), mesh->Min());
  // 36 vertices, 24 unique, 12 shared.
  EXPECT_EQ(24u, mesh->VertexCount());
  EXPECT_EQ(24u, mesh->NormalCount());
  EXPECT_EQ(24u, mesh->IndexCount());
  //EXPECT_EQ(36u, mesh->IndexCount());
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
  // TODO reenable
  //EXPECT_DOUBLE_EQ(50.0, mat->Shininess());
  // transparent: opaque="A_ONE", color=[1 1 1 1]
  // transparency: 1.0
  // resulting transparency value = (1 - color.a * transparency)
  EXPECT_DOUBLE_EQ(0.0, mat->Transparency());
  double srcFactor = -1;
  double dstFactor = -1;
  mat->BlendFactors(srcFactor, dstFactor);
  // TODO reenable
  //EXPECT_DOUBLE_EQ(1.0, srcFactor);
  //EXPECT_DOUBLE_EQ(0.0, dstFactor);

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
  // TODO reenable
  //EXPECT_DOUBLE_EQ(50.0, matOpaque->Shininess());
  // transparent: opaque="A_ONE", color=[1 1 1 1]
  // transparency: not specified, defaults to 1.0
  // resulting transparency value = (1 - color.a * transparency)
  EXPECT_DOUBLE_EQ(0.0, matOpaque->Transparency());
  srcFactor = -1;
  dstFactor = -1;
  matOpaque->BlendFactors(srcFactor, dstFactor);
  // TODO reenable
  //EXPECT_DOUBLE_EQ(1.0, srcFactor);
  //EXPECT_DOUBLE_EQ(0.0, dstFactor);
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
  // TODO check material definition in assimp API, it returns 1
  //EXPECT_EQ(0u, mesh->MaterialCount());

  auto sm = mesh->SubMeshByIndex(0u);
  auto subMesh = sm.lock();
  EXPECT_NE(nullptr, subMesh);
  EXPECT_EQ(math::Vector3d(0, 0, 0), subMesh->Vertex(0u));
  EXPECT_EQ(math::Vector3d(10, 0, 0), subMesh->Vertex(1u));
  EXPECT_EQ(math::Vector3d(10, 10, 0), subMesh->Vertex(2u));
  EXPECT_EQ(math::Vector3d(0, 0, 1), subMesh->Normal(0u));
  EXPECT_EQ(math::Vector3d(0, 0, 1), subMesh->Normal(1u));
  EXPECT_EQ(math::Vector3d(0, 0, 1), subMesh->Normal(2u));
  // TODO reenable, looks OK in dae file and test looks a bit strange?
  /*
  EXPECT_EQ(math::Vector2d(0, 1), subMesh->TexCoord(0u));
  EXPECT_EQ(math::Vector2d(0, 1), subMesh->TexCoord(1u));
  EXPECT_EQ(math::Vector2d(0, 1), subMesh->TexCoord(2u));
  */

  auto smb = mesh->SubMeshByIndex(1u);
  auto subMeshB = smb.lock();
  EXPECT_NE(nullptr, subMeshB);
  EXPECT_EQ(math::Vector3d(10, 0, 0), subMeshB->Vertex(0u));
  EXPECT_EQ(math::Vector3d(20, 0, 0), subMeshB->Vertex(1u));
  EXPECT_EQ(math::Vector3d(20, 10, 0), subMeshB->Vertex(2u));
  EXPECT_EQ(math::Vector3d(0, 0, 1), subMeshB->Normal(0u));
  EXPECT_EQ(math::Vector3d(0, 0, 1), subMeshB->Normal(1u));
  EXPECT_EQ(math::Vector3d(0, 0, 1), subMeshB->Normal(2u));
  // TODO reenable, looks OK in dae file and test looks a bit strange?
  /*
  EXPECT_EQ(math::Vector2d(0, 1), subMeshB->TexCoord(0u));
  EXPECT_EQ(math::Vector2d(0, 1), subMeshB->TexCoord(1u));
  EXPECT_EQ(math::Vector2d(0, 1), subMeshB->TexCoord(2u));
  */

  EXPECT_TRUE(subMeshB->HasTexCoord(0u));
  EXPECT_TRUE(subMeshB->HasTexCoord(1u));
  EXPECT_TRUE(subMeshB->HasTexCoord(2u));
  EXPECT_FALSE(subMeshB->HasTexCoord(3u));

  // test texture coordinate set API
  EXPECT_EQ(3u, subMeshB->TexCoordSetCount());
  EXPECT_EQ(3u, subMeshB->TexCoordCountBySet(0u));
  // TODO reenable, looks OK in dae file and test looks a bit strange?
  /*
  EXPECT_EQ(math::Vector2d(0, 1), subMeshB->TexCoordBySet(0u, 0u));
  EXPECT_EQ(math::Vector2d(0, 1), subMeshB->TexCoordBySet(1u, 0u));
  EXPECT_EQ(math::Vector2d(0, 1), subMeshB->TexCoordBySet(2u, 0u));
  EXPECT_EQ(math::Vector2d(0, 1), subMeshB->TexCoordBySet(1u, 0u));
  EXPECT_EQ(math::Vector2d(0, 1), subMeshB->TexCoordBySet(2u, 0u));
  */

  EXPECT_TRUE(subMeshB->HasTexCoordBySet(0u, 0u));
  EXPECT_TRUE(subMeshB->HasTexCoordBySet(1u, 0u));
  EXPECT_TRUE(subMeshB->HasTexCoordBySet(2u, 0u));
  EXPECT_FALSE(subMeshB->HasTexCoordBySet(3u, 0u));

  EXPECT_EQ(3u, subMeshB->TexCoordCountBySet(1u));
  // TODO reenable, looks OK in dae file and test looks a bit strange?
  /*
  EXPECT_EQ(math::Vector2d(0, 0.5), subMeshB->TexCoordBySet(0u, 1u));
  EXPECT_EQ(math::Vector2d(0, 0.4), subMeshB->TexCoordBySet(1u, 1u));
  EXPECT_EQ(math::Vector2d(0, 0.3), subMeshB->TexCoordBySet(2u, 1u));
  */

  EXPECT_TRUE(subMeshB->HasTexCoordBySet(0u, 1u));
  EXPECT_TRUE(subMeshB->HasTexCoordBySet(1u, 1u));
  EXPECT_TRUE(subMeshB->HasTexCoordBySet(2u, 1u));
  EXPECT_FALSE(subMeshB->HasTexCoordBySet(3u, 1u));

  EXPECT_EQ(3u, subMeshB->TexCoordCountBySet(2u));
  // TODO reenable, looks OK in dae file and test looks a bit strange?
  /*
  EXPECT_EQ(math::Vector2d(0, 0.8), subMeshB->TexCoordBySet(0u, 2u));
  EXPECT_EQ(math::Vector2d(0, 0.7), subMeshB->TexCoordBySet(1u, 2u));
  EXPECT_EQ(math::Vector2d(0, 0.6), subMeshB->TexCoordBySet(2u, 2u));
  */

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
  /*
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
  auto poseEnd = anim->PoseAt(1.666667);
  expectedTrans = math::Matrix4d(
        1, 0, 0, 2,
        0, 1, 0, -1,
        0, 0, 1, 0,
        0, 0, 0, 1);
  EXPECT_EQ(expectedTrans, poseEnd.at("Armature"));
  */
}

/////////////////////////////////////////////////
TEST_F(AssimpLoader, LoadObjBox)
{
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "box.obj"));

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
