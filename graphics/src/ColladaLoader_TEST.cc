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

#include "test_config.h"
#include "ignition/common/Mesh.hh"
#include "ignition/common/SubMesh.hh"
#include "ignition/common/Material.hh"
#include "ignition/common/ColladaLoader.hh"
#include "ignition/common/Skeleton.hh"
#include "ignition/common/SkeletonAnimation.hh"
#include "test/util.hh"

using namespace ignition;

class ColladaLoader : public ignition::testing::AutoLogFixture { };

/////////////////////////////////////////////////
TEST_F(ColladaLoader, LoadBox)
{
  common::ColladaLoader loader;
  common::Mesh *mesh = loader.Load(
      std::string(PROJECT_SOURCE_PATH) + "/test/data/box.dae");

  EXPECT_STREQ("unknown", mesh->Name().c_str());
  EXPECT_EQ(ignition::math::Vector3d(1, 1, 1), mesh->Max());
  EXPECT_EQ(ignition::math::Vector3d(-1, -1, -1), mesh->Min());
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
  common::Mesh *mesh = loader.Load(
      std::string(PROJECT_SOURCE_PATH) + "/test/data/box.dae");

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
      ignition::math::Vector3d v = subMesh->Vertex(j);
      ignition::math::Vector3d n = subMesh->Normal(j);

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
  common::Mesh *mesh = loader.Load(
      std::string(PROJECT_SOURCE_PATH) + "/test/data/zero_count.dae");
  ASSERT_TRUE(mesh);
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
}

/////////////////////////////////////////////////
TEST_F(ColladaLoader, Material)
{
  common::ColladaLoader loader;
  common::Mesh *mesh = loader.Load(
      std::string(PROJECT_SOURCE_PATH) + "/test/data/box.dae");
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
}

/////////////////////////////////////////////////
TEST_F(ColladaLoader, TexCoordSets)
{
  common::ColladaLoader loader;
  // This triangle mesh has multiple uv sets and vertices separated by
  // line breaks
  common::Mesh *mesh = loader.Load(std::string(PROJECT_SOURCE_PATH) +
      "/test/data/multiple_texture_coordinates_triangle.dae");
  ASSERT_TRUE(mesh);

  EXPECT_EQ(3u, mesh->VertexCount());
  EXPECT_EQ(3u, mesh->NormalCount());
  EXPECT_EQ(3u, mesh->IndexCount());
  EXPECT_EQ(3u, mesh->TexCoordCount());
  EXPECT_EQ(1u, mesh->SubMeshCount());
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
}

/////////////////////////////////////////////////
TEST_F(ColladaLoader, LoadBoxWithAnimationOutsideSkeleton)
{
  common::ColladaLoader loader;
  common::Mesh *mesh = loader.Load(
      std::string(PROJECT_SOURCE_PATH) +
      "/test/data/box_with_animation_outside_skeleton.dae");

  EXPECT_EQ(36u, mesh->IndexCount());
  EXPECT_EQ(1u, mesh->SubMeshCount());
  EXPECT_EQ(1u, mesh->MaterialCount());
  EXPECT_LT(0u, mesh->TexCoordCount());
  common::SkeletonPtr skeleton = mesh->MeshSkeleton();
  ASSERT_EQ(1u, skeleton->AnimationCount());
  common::SkeletonAnimation *anim = skeleton->Animation(0);
  EXPECT_EQ(1u, anim->NodeCount());
  EXPECT_TRUE(anim->HasNode("Armature"));
  EXPECT_NE(nullptr, anim->NodeAnimationByName("Armature"));
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
  common::Mesh *mesh = loader.Load(
      std::string(PROJECT_SOURCE_PATH) +
      "/test/data/box_inst_controller_without_skeleton.dae");

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
  common::Mesh *mesh = loader.Load(
      std::string(PROJECT_SOURCE_PATH) +
      "/test/data/box_multiple_inst_controllers.dae");

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
  common::Mesh *mesh = loader.Load(
      std::string(PROJECT_SOURCE_PATH) +
      "/test/data/box_nested_animation.dae");

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
  EXPECT_NE(nullptr, anim->NodeAnimationByName("Bone"));
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
  common::Mesh *mesh = loader.Load(
      std::string(PROJECT_SOURCE_PATH) +
      "/test/data/box_with_default_stride.dae");

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
  common::Mesh *mesh = loader.Load(
      std::string(PROJECT_SOURCE_PATH) +
      "/test/data/box_with_multiple_geoms.dae");

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
TEST_F(ColladaLoader, MergeBoxWithDoubleSkeleton)
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

/////////////////////////////////////////////////
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
