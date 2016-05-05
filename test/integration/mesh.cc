/*
 * Copyright (C) 2015 Open Source Robotics Foundation
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

using namespace ignition;

class MeshTest : public ignition::testing::AutoLogFixture { };

std::string asciiSTLBox =
"solid MYSOLID\n" +
"  facet normal  0.0   0.0  -1.0\n" +
"    outer loop\n" +
"      vertex    0.0   0.0   0.0\n"
"      vertex    1.0   1.0   0.0\n" +
"      vertex    1.0   0.0   0.0\n" +
"    endloop\n" +
"  endfacet\n" +
"  facet normal  0.0   0.0  -1.0\n" +
"    outer loop\n" +
"      vertex    0.0   0.0   0.0\n" +
"      vertex    0.0   1.0   0.0\n" +
"      vertex    1.0   1.0   0.0\n" +
"    endloop\n" +
"  endfacet\n" +
"  facet normal -1.0   0.0   0.0\n" +
"    outer loop\n" +
"      vertex    0.0   0.0   0.0\n" +
"      vertex    0.0   1.0   1.0\n" +
"      vertex    0.0   1.0   0.0\n" +
"    endloop\n" +
"  endfacet\n" +
"  facet normal -1.0   0.0   0.0\n" +
"    outer loop\n" +
"      vertex    0.0   0.0   0.0\n" +
"      vertex    0.0   0.0   1.0\n" +
"      vertex    0.0   1.0   1.0\n" +
"    endloop\n" +
"  endfacet\n" +
"  facet normal  0.0   1.0   0.0\n" +
"    outer loop\n" +
"      vertex    0.0   1.0   0.0\n" +
"      vertex    1.0   1.0   1.0\n" +
"      vertex    1.0   1.0   0.0\n" +
"    endloop\n" +
"  endfacet\n" +
"  facet normal  0.0   1.0   0.0\n" +
"    outer loop\n" +
"      vertex    0.0   1.0   0.0\n" +
"      vertex    0.0   1.0   1.0\n" +
"      vertex    1.0   1.0   1.0\n" +
"    endloop\n" +
"  endfacet\n" +
"  facet normal  1.0   0.0   0.0\n" +
"    outer loop\n" +
"      vertex    1.0   0.0   0.0\n" +
"      vertex    1.0   1.0   0.0\n" +
"      vertex    1.0   1.0   1.0\n" +
"    endloop\n" +
"  endfacet\n" +
"  facet normal  1.0   0.0   0.0\n" +
"    outer loop\n" +
"      vertex    1.0   0.0   0.0\n" +
"      vertex    1.0   1.0   1.0\n" +
"      vertex    1.0   0.0   1.0\n" +
"    endloop\n" +
"  endfacet\n" +
"  facet normal  0.0  -1.0   0.0\n" +
"    outer loop\n" +
"      vertex    0.0   0.0   0.0\n" +
"      vertex    1.0   0.0   0.0\n" +
"      vertex    1.0   0.0   1.0\n" +
"    endloop\n" +
"  endfacet\n" +
"  facet normal  0.0  -1.0   0.0\n" +
"    outer loop\n" +
"      vertex    0.0   0.0   0.0\n" +
"      vertex    1.0   0.0   1.0\n" +
"      vertex    0.0   0.0   1.0\n" +
"    endloop\n" +
"  endfacet\n" +
"  facet normal  0.0   0.0   1.0\n" +
"    outer loop\n" +
"      vertex    0.0   0.0   1.0\n" +
"      vertex    1.0   0.0   1.0\n" +
"      vertex    1.0   1.0   1.0\n" +
"    endloop\n" +
"  endfacet\n" +
"  facet normal  0.0   0.0   1.0\n" +
"    outer loop\n" +
"      vertex    0.0   0.0   1.0\n" +
"      vertex    1.0   1.0   1.0\n" +
"      vertex    0.0   1.0   1.0\n" +
"    endloop\n" +
"  endfacet\n" +
"endsolid MYSOLID";


/////////////////////////////////////////////////
TEST_F(MeshTest, Mesh)
{
  // Cleanup test directory.
  common::SystemPaths *paths = common::SystemPaths::Instance();
  ignition::common::removeAll(paths->DefaultTestPath());
  ignition::common::createDirectories(paths->DefaultTestPath());

  EXPECT_EQ(NULL, common::MeshManager::Instance()->Load("break.mesh"));
  EXPECT_EQ(NULL, common::MeshManager::Instance()->Load("break.3ds"));
  EXPECT_EQ(NULL, common::MeshManager::Instance()->Load("break.xml"));

  const common::Mesh *mesh =
    common::MeshManager::Instance()->Mesh("unit_box");
  EXPECT_EQ(static_cast<unsigned int>(24), mesh->VertexCount());
  EXPECT_EQ(static_cast<unsigned int>(24), mesh->NormalCount());
  EXPECT_EQ(static_cast<unsigned int>(36), mesh->IndexCount());
  EXPECT_EQ(static_cast<unsigned int>(24), mesh->TexCoordCount());
  EXPECT_EQ(static_cast<unsigned int>(0), mesh->MaterialCount());

  ignition::math::Vector3d center, min, max;
  mesh->AABB(center, min, max);
  EXPECT_TRUE(center == ignition::math::Vector3d(0, 0, 0));
  EXPECT_TRUE(min == ignition::math::Vector3d(-.5, -.5, -.5));
  EXPECT_TRUE(max == ignition::math::Vector3d(.5, .5, .5));

  float *vertArray = NULL;
  int *indArray = NULL;
  mesh->FillArrays(&vertArray, &indArray);

  int i = 0;
  EXPECT_FLOAT_EQ(.5, vertArray[i++]);
  EXPECT_FLOAT_EQ(-.5, vertArray[i++]);
  EXPECT_FLOAT_EQ(.5, vertArray[i++]);

  EXPECT_FLOAT_EQ(-.5, vertArray[i++]);
  EXPECT_FLOAT_EQ(-.5, vertArray[i++]);
  EXPECT_FLOAT_EQ(.5, vertArray[i++]);

  EXPECT_FLOAT_EQ(-.5, vertArray[i++]);
  EXPECT_FLOAT_EQ(-.5, vertArray[i++]);
  EXPECT_FLOAT_EQ(-.5, vertArray[i++]);

  EXPECT_FLOAT_EQ(.5, vertArray[i++]);
  EXPECT_FLOAT_EQ(-.5, vertArray[i++]);
  EXPECT_FLOAT_EQ(-.5, vertArray[i++]);

  EXPECT_FLOAT_EQ(-.5, vertArray[i++]);
  EXPECT_FLOAT_EQ(.5, vertArray[i++]);
  EXPECT_FLOAT_EQ(.5, vertArray[i++]);

  EXPECT_FLOAT_EQ(.5, vertArray[i++]);
  EXPECT_FLOAT_EQ(.5, vertArray[i++]);
  EXPECT_FLOAT_EQ(.5, vertArray[i++]);

  EXPECT_FLOAT_EQ(.5, vertArray[i++]);
  EXPECT_FLOAT_EQ(.5, vertArray[i++]);
  EXPECT_FLOAT_EQ(-.5, vertArray[i++]);

  EXPECT_FLOAT_EQ(-.5, vertArray[i++]);
  EXPECT_FLOAT_EQ(.5, vertArray[i++]);
  EXPECT_FLOAT_EQ(-.5, vertArray[i++]);

  common::Mesh *newMesh = new common::Mesh();
  newMesh->SetName("testBox");
  common::SubMesh *subMesh = new common::SubMesh();
  newMesh->AddSubMesh(subMesh);

  std::vector<ignition::math::Vector3d> verts;
  std::vector<ignition::math::Vector3d> norms;

  EXPECT_THROW(mesh->SubMesh(1), common::Exception);

  for (i = 0; i < 24; ++i)
  {
    verts.push_back(mesh->SubMesh(0)->Vertex(i));
    norms.push_back(mesh->SubMesh(0)->Normal(i));
  }

  subMesh->CopyVertices(verts);
  subMesh->CopyNormals(norms);
  EXPECT_TRUE(subMesh->HasVertex(ignition::math::Vector3d(-.5, -.5, -.5)));
  EXPECT_FALSE(subMesh->HasVertex(ignition::math::Vector3d(0, 0, 0)));

  newMesh->AABB(center, min, max);
  EXPECT_TRUE(center == ignition::math::Vector3d(0, 0, 0));
  EXPECT_TRUE(min == ignition::math::Vector3d(-.5, -.5, -.5));
  EXPECT_TRUE(max == ignition::math::Vector3d(.5, .5, .5));

  subMesh->SetVertexCount(1);
  subMesh->SetIndexCount(1);
  subMesh->SetNormalCount(1);
  subMesh->SetTexCoordCount(1);

  EXPECT_EQ(static_cast<unsigned int>(1), subMesh->VertexCount());
  EXPECT_EQ(static_cast<unsigned int>(1), subMesh->IndexCount());
  EXPECT_EQ(static_cast<unsigned int>(1), subMesh->NormalCount());
  EXPECT_EQ(static_cast<unsigned int>(1), subMesh->TexCoordCount());

  subMesh->SetVertex(0, ignition::math::Vector3d(1, 2, 3));
  EXPECT_TRUE(subMesh->Vertex(0) == ignition::math::Vector3d(1, 2, 3));

  subMesh->SetTexCoord(0, ignition::math::Vector2d(.1, .2));
  EXPECT_TRUE(subMesh->TexCoord(0) == ignition::math::Vector2d(.1, .2));

  newMesh->GenSphericalTexCoord(ignition::math::Vector3d(0, 0, 0));
  delete newMesh;

  std::ofstream stlFile((paths->DefaultTestPath() +
      "/ignition_stl_test.stl").c_str(), std::ios::out);
  stlFile << asciiSTLBox;
  stlFile.close();

  mesh = common::MeshManager::Instance()->Load(
      paths->DefaultTestPath() + "/ignition_stl_test-bad.stl");
  EXPECT_EQ(NULL, mesh);

  mesh = common::MeshManager::Instance()->Load(
      paths->DefaultTestPath() + "/ignition_stl_test.stl");
  mesh->AABB(center, min, max);
  EXPECT_TRUE(center == ignition::math::Vector3d(0.5, 0.5, 0.5));
  EXPECT_TRUE(min == ignition::math::Vector3d(0, 0, 0));
  EXPECT_TRUE(max == ignition::math::Vector3d(1, 1, 1));

  // Cleanup test directory.
  igntion::common::removeAll(paths->DefaultTestPath());
}

/////////////////////////////////////////////////
// Test centering a submesh.
TEST_F(MeshTest, MeshMove)
{
  common::ColladaLoader loader;
  common::Mesh *mesh = loader.Load(
      std::string(PROJECT_SOURCE_PATH) + "/test/data/box_offset.dae");

  // The default location of the box_offset is not centered
  EXPECT_EQ(ignition::math::Vector3d(5.46554, 2.18039, 4.8431), mesh->Max());
  EXPECT_EQ(ignition::math::Vector3d(3.46555, 0.180391, 2.8431), mesh->Min());

  mesh->Center(ignition::math::Vector3d::Zero);

  EXPECT_EQ(ignition::math::Vector3d(1.0, 1.0, 1.0), mesh->Max());
  EXPECT_EQ(ignition::math::Vector3d(-1.0, -1.0, -1.0), mesh->Min());

  mesh->Translate(ignition::math::Vector3d(1, 2, 3));
  EXPECT_EQ(ignition::math::Vector3d(2.0, 3.0, 4.0), mesh->Max());
  EXPECT_EQ(ignition::math::Vector3d(0.0, 1.0, 2.0), mesh->Min());
}

/////////////////////////////////////////////////
// Test centering a submesh.
TEST_F(MeshTest, SubMeshCenter)
{
  common::ColladaLoader loader;
  common::Mesh *mesh = loader.Load(
      std::string(PROJECT_SOURCE_PATH) + "/test/data/box_offset.dae");

  // The default location of the box_offest is not centered
  EXPECT_EQ(ignition::math::Vector3d(5.46554, 2.18039, 4.8431), mesh->Max());
  EXPECT_EQ(ignition::math::Vector3d(3.46555, 0.180391, 2.8431), mesh->Min());

  // Get the Cube submesh
  common::SubMesh submesh(mesh->SubMesh("Cube"));

  submesh.Center(ignition::math::Vector3d(1, 2, 3));
  EXPECT_EQ(ignition::math::Vector3d(0, 1, 2), submesh.Min());
  EXPECT_EQ(ignition::math::Vector3d(2, 3, 4), submesh.Max());

  submesh.Translate(ignition::math::Vector3d(1, 2, 3));
  EXPECT_EQ(ignition::math::Vector3d(1, 3, 5), submesh.Min());
  EXPECT_EQ(ignition::math::Vector3d(3, 5, 7), submesh.Max());

  // The original mesh should not change
  EXPECT_EQ(ignition::math::Vector3d(5.46554, 2.18039, 4.8431), mesh->Max());
  EXPECT_EQ(ignition::math::Vector3d(3.46555, 0.180391, 2.8431), mesh->Min());
}

/////////////////////////////////////////////////
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
