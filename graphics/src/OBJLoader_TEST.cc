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
#include "gz/common/OBJLoader.hh"

#include "gz/common/testing/AutoLogFixture.hh"
#include "gz/common/testing/TestPaths.hh"

using namespace gz;

class OBJLoaderTest : public common::testing::AutoLogFixture { };

/////////////////////////////////////////////////
// This tests opening an OBJ file that has PBR fields
TEST_F(OBJLoaderTest, PBR)
{
  gz::common::OBJLoader objLoader;

  // load obj file exported by 3ds max that has pbr extension
  {
    std::string meshFilename =
      common::testing::TestFile("data", "cube_pbr.obj");

    gz::common::Mesh *mesh = objLoader.Load(meshFilename);
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
    delete mesh;
  }

  // load obj file exported by blender - it shoves pbr maps into
  // existing fields
  {
    std::string meshFilename =
      common::testing::TestFile("data", "blender_pbr.obj");

    gz::common::Mesh *mesh = objLoader.Load(meshFilename);
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
    delete mesh;
  }
}
