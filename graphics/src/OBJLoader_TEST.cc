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
#include "ignition/common/OBJLoader.hh"
#include "test/util.hh"

using namespace ignition;

class OBJLoaderTest : public ignition::testing::AutoLogFixture { };

/////////////////////////////////////////////////
TEST_F(OBJLoaderTest, LoadObjBox)
{
  common::OBJLoader loader;
  common::Mesh *mesh = loader.Load(
      std::string(PROJECT_SOURCE_PATH) + "/test/data/box.obj");

  EXPECT_STREQ("unknown", mesh->Name().c_str());
  EXPECT_EQ(ignition::math::Vector3d(1, 1, 1), mesh->Max());
  EXPECT_EQ(ignition::math::Vector3d(-1, -1, -1), mesh->Min());
  // 36 vertices
  EXPECT_EQ(36u, mesh->VertexCount());
  EXPECT_EQ(36u, mesh->NormalCount());
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
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
