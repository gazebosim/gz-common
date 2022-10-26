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

#include "test_config.h"

#include "gz/common/Mesh.hh"
#include "gz/common/SubMesh.hh"
#include "gz/common/MeshCSG.hh"
#include "gz/common/STLLoader.hh"

using namespace ignition;
using namespace common;

class MeshCSGTest : public common::testing::AutoLogFixture { };

/////////////////////////////////////////////////
TEST_F(MeshCSGTest, MeshCSG)
{
  #ifdef _WIN32
  std::cerr << "Skipping test on windows due to issue #468" << std::endl;
  SKIP_TEST();
  #endif

  common::STLLoader loader;

  auto mesh1 = loader.Load(
      common::testing::TestFile("data", "cube.stl"));
  EXPECT_NE(nullptr, mesh1);
  auto mesh2 = loader.Load(
      common::testing::TestFile("data", "cube.stl"));
  EXPECT_NE(nullptr, mesh2);
  mesh2->Translate(math::Vector3d(10, 0, 0));

  MeshCSG meshCSG;
  auto mesh = meshCSG.CreateBoolean(mesh1, mesh2, MeshCSG::DIFFERENCE);

  EXPECT_STREQ("unknown", mesh->Name().c_str());
  EXPECT_EQ(math::Vector3d(20, 0, 20), mesh->Max());
  EXPECT_EQ(math::Vector3d(0, -20, 0), mesh->Min());
  // 36 vertices, 24 unique, 12 shared.
  EXPECT_LT(0u, mesh->VertexCount());
  EXPECT_EQ(0u, mesh->NormalCount());
  EXPECT_LT(0u, mesh->IndexCount());
  EXPECT_EQ(0u, mesh->TexCoordCount());
  EXPECT_EQ(1u, mesh->SubMeshCount());
  EXPECT_EQ(0u, mesh->MaterialCount());

  auto sm = mesh->SubMeshByIndex(0u);
  auto subMesh = sm.lock();
  EXPECT_NE(nullptr, subMesh);

  mesh = meshCSG.CreateBoolean(mesh1, mesh2, MeshCSG::UNION);

  EXPECT_STREQ("unknown", mesh->Name().c_str());
  EXPECT_EQ(math::Vector3d(30, 0, 20), mesh->Max());
  EXPECT_EQ(math::Vector3d(0, -20, 0), mesh->Min());
  // 36 vertices, 24 unique, 12 shared.
  EXPECT_LT(0u, mesh->VertexCount());
  EXPECT_EQ(0u, mesh->NormalCount());
  EXPECT_LT(0u, mesh->IndexCount());
  EXPECT_EQ(0u, mesh->TexCoordCount());
  EXPECT_EQ(1u, mesh->SubMeshCount());
  EXPECT_EQ(0u, mesh->MaterialCount());

  sm = mesh->SubMeshByIndex(0u);
  subMesh = sm.lock();
  EXPECT_NE(nullptr, subMesh);

  mesh = meshCSG.CreateBoolean(mesh1, mesh2, MeshCSG::INTERSECTION);

  EXPECT_STREQ("unknown", mesh->Name().c_str());
  EXPECT_EQ(math::Vector3d(20, 0, 20), mesh->Max());
  EXPECT_EQ(math::Vector3d(10, -20, 0), mesh->Min());
  // 36 vertices, 24 unique, 12 shared.
  EXPECT_LT(0u, mesh->VertexCount());
  EXPECT_EQ(0u, mesh->NormalCount());
  EXPECT_LT(0u, mesh->IndexCount());
  EXPECT_EQ(0u, mesh->TexCoordCount());
  EXPECT_EQ(1u, mesh->SubMeshCount());
  EXPECT_EQ(0u, mesh->MaterialCount());

  sm = mesh->SubMeshByIndex(0u);
  subMesh = sm.lock();
  EXPECT_NE(nullptr, subMesh);
}
