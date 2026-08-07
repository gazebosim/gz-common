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
  // Depends on fix in assimp main branch for nested animation naming
  // TODO(luca) Fix is merged in assimp main, add when it is re-released
  // EXPECT_EQ(anim->Name(), "Armature");
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
// This tests opening an OBJ file that has an invalid material reference
TEST_F(AssimpLoader, ObjInvalidMaterial)
{
  common::AssimpLoader loader;

  std::string meshFilename =
    common::testing::TestFile("data", "invalid_material.obj");

  common::Mesh *mesh = loader.Load(meshFilename);

  EXPECT_TRUE(mesh != nullptr);
  delete mesh;
}

/////////////////////////////////////////////////
// Open a non existing file
TEST_F(AssimpLoader, NonExistingMesh)
{
  common::AssimpLoader loader;

  std::string meshFilename =
    common::testing::TestFile("data", "non_existing_mesh.glb");

  common::Mesh *mesh = loader.Load(meshFilename);

  EXPECT_EQ(mesh->SubMeshCount(), 0);
  delete mesh;
}

/////////////////////////////////////////////////
// This test opens a FBX file
TEST_F(AssimpLoader, LoadFbxBox)
{
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "box.fbx"));

  EXPECT_STREQ("unknown", mesh->Name().c_str());
  EXPECT_EQ(math::Vector3d(100, 100, 100), mesh->Max());
  EXPECT_EQ(math::Vector3d(-100, -100, -100), mesh->Min());

  EXPECT_EQ(24u, mesh->VertexCount());
  EXPECT_EQ(24u, mesh->NormalCount());
  EXPECT_EQ(36u, mesh->IndexCount());
  EXPECT_EQ(24u, mesh->TexCoordCount());
  EXPECT_EQ(1u, mesh->SubMeshCount());
  EXPECT_EQ(1u, mesh->MaterialCount());

  // Make sure we can read the submesh name
  EXPECT_STREQ("Cube", mesh->SubMeshByIndex(0).lock()->Name().c_str());

  EXPECT_EQ(mesh->MaterialCount(), 1u);

  const common::MaterialPtr mat = mesh->MaterialByIndex(0u);
  ASSERT_TRUE(mat.get());

  // Make sure we read the material color values
  EXPECT_EQ(mat->Ambient(), math::Color(0.0f, 0.0f, 0.0f, 1.0f));
  EXPECT_EQ(mat->Diffuse(), math::Color(0.8f, 0.8f, 0.8f, 1.0f));
  EXPECT_EQ(mat->Specular(), math::Color(0.8f, 0.8f, 0.8f, 1.0f));
  EXPECT_DOUBLE_EQ(mat->Transparency(), 0.0);
  delete mesh;
}

/////////////////////////////////////////////////
// This test opens a GLB file
TEST_F(AssimpLoader, LoadGlTF2Box)
{
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "box.glb"));

  EXPECT_STREQ("unknown", mesh->Name().c_str());
  EXPECT_EQ(math::Vector3d(1, 1, 1), mesh->Max());
  EXPECT_EQ(math::Vector3d(-1, -1, -1), mesh->Min());

  EXPECT_EQ(24u, mesh->VertexCount());
  EXPECT_EQ(24u, mesh->NormalCount());
  EXPECT_EQ(36u, mesh->IndexCount());
  EXPECT_EQ(24u, mesh->TexCoordCount());
  EXPECT_EQ(1u, mesh->SubMeshCount());
  EXPECT_EQ(1u, mesh->MaterialCount());

  // Make sure we can read the submesh name
  EXPECT_STREQ("Cube", mesh->SubMeshByIndex(0).lock()->Name().c_str());

  EXPECT_EQ(mesh->MaterialCount(), 1u);

  const common::MaterialPtr mat = mesh->MaterialByIndex(0u);
  ASSERT_TRUE(mat.get());

  // Make sure we read the material color values
  EXPECT_EQ(mat->Ambient(), math::Color(0.4f, 0.4f, 0.4f, 1.0f));
  EXPECT_EQ(mat->Diffuse(), math::Color(0.8f, 0.8f, 0.8f, 1.0f));
  EXPECT_EQ(mat->Specular(), math::Color(0.0f, 0.0f, 0.0f, 1.0f));
  EXPECT_DOUBLE_EQ(mat->Transparency(), 0.0);
  delete mesh;
}

/////////////////////////////////////////////////
// Open a gltf mesh with an external texture
TEST_F(AssimpLoader, LoadGlTF2BoxExternalTexture)
{
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "gltf", "PurpleCube.gltf"));

  EXPECT_STREQ("unknown", mesh->Name().c_str());

  // Make sure we can read the submesh name
  EXPECT_STREQ("PurpleCube", mesh->SubMeshByIndex(0).lock()->Name().c_str());

  EXPECT_EQ(mesh->MaterialCount(), 1u);

  const common::MaterialPtr mat = mesh->MaterialByIndex(0u);
  ASSERT_TRUE(mat.get());
  // Data is now loaded in memory
  EXPECT_NE(nullptr, mat->TextureData());
  auto testTextureFile =
    common::testing::TestFile("data/gltf", "PurpleCube_Diffuse.png");
  EXPECT_EQ(testTextureFile + "_Diffuse", mat->TextureImage());
  delete mesh;
}

/////////////////////////////////////////////////
// Open a gltf mesh with transmission extension
TEST_F(AssimpLoader, LoadGlTF2BoxTransmission)
{
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "box_transmission.glb"));

  EXPECT_STREQ("unknown", mesh->Name().c_str());

  // Make sure we can read the submesh name
  EXPECT_STREQ("Cube", mesh->SubMeshByIndex(0).lock()->Name().c_str());

  EXPECT_EQ(mesh->MaterialCount(), 1u);

  const common::MaterialPtr mat = mesh->MaterialByIndex(0u);
  ASSERT_TRUE(mat.get());
  // transmission currently modeled as transparency
  EXPECT_FLOAT_EQ(0.1f, mat->Transparency());
  delete mesh;
}

/////////////////////////////////////////////////
// This test loads a box glb mesh with embedded compressed jpeg texture
TEST_F(AssimpLoader, LoadGlTF2BoxWithJPEGTexture)
{
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "box_texture_jpg.glb"));

  EXPECT_STREQ("unknown", mesh->Name().c_str());
  EXPECT_EQ(math::Vector3d(1, 1, 1), mesh->Max());
  EXPECT_EQ(math::Vector3d(-1, -1, -1), mesh->Min());

  EXPECT_EQ(24u, mesh->VertexCount());
  EXPECT_EQ(24u, mesh->NormalCount());
  EXPECT_EQ(36u, mesh->IndexCount());
  EXPECT_EQ(24u, mesh->TexCoordCount());
  EXPECT_EQ(1u, mesh->SubMeshCount());
  EXPECT_EQ(1u, mesh->MaterialCount());

  // Make sure we can read the submesh name
  EXPECT_STREQ("Cube", mesh->SubMeshByIndex(0).lock()->Name().c_str());

  const common::MaterialPtr mat = mesh->MaterialByIndex(0u);
  ASSERT_TRUE(mat.get());

  // Make sure we read the material color values
  EXPECT_EQ(math::Color(0.4f, 0.4f, 0.4f, 1.0f), mat->Ambient());
  EXPECT_EQ(math::Color(1.0f, 1.0f, 1.0f, 1.0f), mat->Diffuse());
  EXPECT_EQ(math::Color(0.0f, 0.0f, 0.0f, 1.0f), mat->Specular());
  // Use the new globally unique canonical name format
  std::string expectedName = common::testing::TestFile("data",
      "box_texture_jpg.glb") + "#*0_Diffuse";
  EXPECT_EQ(expectedName, mat->TextureImage());
  EXPECT_NE(nullptr, mat->TextureData());
  delete mesh;
}

/////////////////////////////////////////////////
// Use a fully featured glb test asset, including PBR textures, emissive maps
// embedded textures, lightmaps, animations to test advanced glb features
TEST_F(AssimpLoader, LoadGlbPbrAsset)
{
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "fully_featured.glb"));

  EXPECT_STREQ("unknown", mesh->Name().c_str());

  EXPECT_EQ(mesh->SubMeshCount(), 7);
  EXPECT_STREQ("Floor", mesh->SubMeshByIndex(0).lock()->Name().c_str());
  EXPECT_STREQ("SquareShelf", mesh->SubMeshByIndex(1).lock()->Name().c_str());
  EXPECT_STREQ("OpenRoboticsLogo.002",
      mesh->SubMeshByIndex(2).lock()->Name().c_str());
  EXPECT_STREQ("OpenRoboticsLogo.001",
      mesh->SubMeshByIndex(3).lock()->Name().c_str());
  EXPECT_STREQ("EmissiveCube", mesh->SubMeshByIndex(4).lock()->Name().c_str());
  EXPECT_STREQ("OpenCola", mesh->SubMeshByIndex(5).lock()->Name().c_str());
  EXPECT_STREQ("OpenRoboticsLogo",
      mesh->SubMeshByIndex(6).lock()->Name().c_str());

  // Emissive cube has an embedded emissive texture
  auto materialId = mesh->SubMeshByIndex(4).lock()->GetMaterialIndex();
  ASSERT_TRUE(materialId.has_value());
  auto material = mesh->MaterialByIndex(materialId.value());
  ASSERT_NE(material, nullptr);
  auto pbr = material->PbrMaterial();
  ASSERT_NE(pbr, nullptr);
  EXPECT_NE(pbr->EmissiveMapData(), nullptr);

  // SquareShelf has full PBR textures, including metallicroughness
  // and ambient occlusion
  materialId = mesh->SubMeshByIndex(1).lock()->GetMaterialIndex();
  ASSERT_TRUE(materialId.has_value());
  material = mesh->MaterialByIndex(materialId.value());
  ASSERT_NE(material, nullptr);
  pbr = material->PbrMaterial();
  ASSERT_NE(pbr, nullptr);

  // Check the texture data itself
  auto img = material->TextureData();
  ASSERT_NE(img, nullptr);
  EXPECT_EQ(img->Width(), 512);
  EXPECT_EQ(img->Height(), 512);
  // A black and a white pixel
  EXPECT_EQ(img->Pixel(0, 0), math::Color(0.0f, 0.0f, 0.0f, 1.0f));
  EXPECT_EQ(img->Pixel(100, 100), math::Color(1.0f, 1.0f, 1.0f, 1.0f));

  EXPECT_NE(pbr->NormalMapData(), nullptr);
  // Metallic roughness and alpha from textures only works in assimp > 5.2.0
  // Alpha from textures
  EXPECT_TRUE(material->TextureAlphaEnabled());
  EXPECT_TRUE(material->TwoSidedEnabled());
  EXPECT_EQ(material->AlphaThreshold(), 0.5);
  // Metallic and roughness maps
  EXPECT_NE(pbr->MetalnessMapData(), nullptr);
  EXPECT_NE(pbr->RoughnessMapData(), nullptr);
  // Check pixel values to test metallicroughness texture splitting
  EXPECT_FLOAT_EQ(pbr->MetalnessMapData()->Pixel(256, 256).R(), 0.0f);
  EXPECT_FLOAT_EQ(pbr->RoughnessMapData()->Pixel(256, 256).R(),
                  124.0f / 255.0f);

  // Bug in assimp 5.0.x that doesn't parse coordinate sets properly
  // \todo(iche033) Lightmaps are disabled for glb meshes
  // due to upstream bug
  // EXPECT_EQ(pbr->LightMapTexCoordSet(), 1);

  // \todo(iche033) Lightmaps are disabled for glb meshes
  // due to upstream bug
  // EXPECT_NE(pbr->LightMapData(), nullptr);

  // Mesh has 3 animations
  auto skel = mesh->MeshSkeleton();
  ASSERT_NE(skel, nullptr);
  ASSERT_EQ(skel->AnimationCount(), 3);
  EXPECT_STREQ("Action1", skel->Animation(0)->Name().c_str());
  EXPECT_STREQ("Action2", skel->Animation(1)->Name().c_str());
  EXPECT_STREQ("Action3", skel->Animation(2)->Name().c_str());
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
