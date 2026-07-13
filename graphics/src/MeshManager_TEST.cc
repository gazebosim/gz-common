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

#include "gz/common/Material.hh"
#include "gz/common/Mesh.hh"
#include "gz/common/Skeleton.hh"
#include "gz/common/SkeletonAnimation.hh"
#include "gz/common/SubMesh.hh"
#include "gz/common/MeshManager.hh"

#include "gz/common/testing/AutoLogFixture.hh"
#include "gz/common/testing/TestPaths.hh"

using namespace gz;

#ifndef _WIN32
class MeshManager : public common::testing::AutoLogFixture,
                    public testing::WithParamInterface<bool> {
  protected: void SetUp() override
  {
    common::testing::AutoLogFixture::SetUp();

    forceAssimpEnv = this->GetParam();
    if (forceAssimpEnv)
    {
      common::setenv("GZ_MESH_FORCE_ASSIMP", "true");
    }
    else
    {
      common::setenv("GZ_MESH_FORCE_ASSIMP", "false");
    }
    common::MeshManager::Instance()->SetAssimpEnvs();
  }

  protected: void TearDown() override
  {
    common::unsetenv("GZ_MESH_FORCE_ASSIMP");
    common::MeshManager::Instance()->SetAssimpEnvs();
    common::testing::AutoLogFixture::TearDown();
  }

  protected: bool forceAssimpEnv = false;
};

INSTANTIATE_TEST_SUITE_P(
    ForceAssimpScenarios,
    MeshManager,
    testing::Bool());

/////////////////////////////////////////////////
TEST_P(MeshManager, CreateExtrudedPolyline)
{
  // test extrusion of a path with two subpaths:
  // a smaller square inside a bigger square.
  // The smaller square should be treated as a hole inside the bigger square.
  std::vector<std::vector<gz::math::Vector2d> > path;
  std::vector<gz::math::Vector2d> subpath01;
  subpath01.emplace_back(0, 0);
  subpath01.emplace_back(1, 0);
  subpath01.emplace_back(1, 1);
  subpath01.emplace_back(0, 1);
  subpath01.emplace_back(0, 0);

  std::vector<gz::math::Vector2d> subpath02;
  subpath02.emplace_back(0.25, 0.25);
  subpath02.emplace_back(0.25, 0.75);
  subpath02.emplace_back(0.75, 0.75);
  subpath02.emplace_back(0.75, 0.25);
  subpath02.emplace_back(0.25, 0.25);

  path.push_back(subpath01);
  path.push_back(subpath02);

  std::string meshName = "extruded_path";
  double height = 10.0;
  common::MeshManager::Instance()->CreateExtrudedPolyline(
      meshName, path, height);

  // check mesh
  EXPECT_TRUE(common::MeshManager::Instance()->HasMesh(meshName));
  const common::Mesh *mesh =
    common::MeshManager::Instance()->MeshByName(meshName);
  EXPECT_TRUE(mesh != nullptr);

  unsigned int submeshCount = mesh->SubMeshCount();
  EXPECT_EQ(submeshCount, 1u);

  // check submesh bounds
  auto submesh = mesh->SubMeshByIndex(0).lock();
  EXPECT_TRUE(submesh != nullptr);
  EXPECT_EQ(gz::math::Vector3d(0, 0, 0), submesh->Min());
  EXPECT_EQ(gz::math::Vector3d(1.0, 1.0, 10.0), submesh->Max());

  // check vertices
  for (unsigned int i = 0; i < submesh->VertexCount(); ++i)
  {
    gz::math::Vector3d v = submesh->Vertex(i);

    // check no vertices are in the region of the hole
    EXPECT_FALSE((v.X() > 0.25 && v.X() < 0.75));
    EXPECT_FALSE((v.Y() > 0.25 && v.Y() < 0.75));

    // check extruded height
    EXPECT_TRUE((gz::math::equal(v.Z(), 0.0) ||
          gz::math::equal(v.Z(), 10.0)));
  }

  // verify same number of normals and vertices
  EXPECT_EQ(submesh->VertexCount(), submesh->NormalCount());

  // check normals
  for (unsigned int i = 0; i < submesh->NormalCount(); ++i)
  {
    gz::math::Vector3d v = submesh->Vertex(i);
    gz::math::Vector3d n = submesh->Normal(i);

    // vertex at 0 could be a bottom face or side face
    if (gz::math::equal(v.Z(), 0.0))
    {
      if (gz::math::equal(n.Z(), 0.0))
      {
        // side face - check non-zero normal
        EXPECT_TRUE(!(gz::math::equal(n.X(), 0.0) &&
              gz::math::equal(n.Y(), 0.0)));
      }
      else
      {
        // bottom face - normal in -z direction
        EXPECT_TRUE((n == -gz::math::Vector3d::UnitZ) ||
            (gz::math::equal(n.Z(), 0.0)));
      }
    }

    // vertex at height could be a top face or side face
    if (gz::math::equal(v.Z(), 10.0))
    {
      if (gz::math::equal(n.Z(), 0.0))
      {
        // side face - check non-zero normal
        EXPECT_TRUE(!(gz::math::equal(n.X(), 0.0) &&
              gz::math::equal(n.Y(), 0.0)));
      }
      else
      {
        // top face - normal in +z direction
        EXPECT_TRUE((n == gz::math::Vector3d::UnitZ) ||
            (gz::math::equal(n.Z(), 0.0)));
      }
    }
  }
}

/////////////////////////////////////////////////
TEST_P(MeshManager, CreateExtrudedPolylineClosedPath)
{
  // test extrusion of a path that has two closed subpaths, i.e.,
  // first and last vertices are the same.
  // The following two subpaths form the letter 'A'.
  std::vector<std::vector<gz::math::Vector2d> > path2;
  std::vector<gz::math::Vector2d> subpath03;
  subpath03.emplace_back(2.27467, 1.0967);
  subpath03.emplace_back(1.81094, 2.35418);
  subpath03.emplace_back(2.74009, 2.35418);

  std::vector<gz::math::Vector2d> subpath04;
  subpath04.emplace_back(2.08173, 0.7599);
  subpath04.emplace_back(2.4693, 0.7599);
  subpath04.emplace_back(3.4323, 3.28672);
  subpath04.emplace_back(3.07689, 3.28672);
  subpath04.emplace_back(2.84672, 2.63851);
  subpath04.emplace_back(1.7077, 2.63851);
  subpath04.emplace_back(1.47753, 3.28672);
  subpath04.emplace_back(1.11704, 3.28672);

  path2.push_back(subpath03);
  path2.push_back(subpath04);

  std::string meshName = "extruded_path_closed";
  double height = 2.0;
  common::MeshManager::Instance()->CreateExtrudedPolyline(
      meshName, path2, height);

  // check mesh
  EXPECT_TRUE(common::MeshManager::Instance()->HasMesh(meshName));
  const common::Mesh *mesh =
    common::MeshManager::Instance()->MeshByName(meshName);
  EXPECT_TRUE(mesh != nullptr);

  unsigned int submeshCount = mesh->SubMeshCount();
  EXPECT_EQ(submeshCount, 1u);

  // check submesh bounds
  auto submesh = mesh->SubMeshByIndex(0).lock();
  EXPECT_TRUE(submesh != nullptr);
  EXPECT_EQ(submesh->Min(), gz::math::Vector3d(1.11704, 0.7599, 0));
  EXPECT_EQ(submesh->Max(), gz::math::Vector3d(3.4323, 3.28672, 2.0));

  for (unsigned int i = 0; i < submesh->VertexCount(); ++i)
  {
    gz::math::Vector3d v = submesh->Vertex(i);

    // check no vertices are in the region of the hole using a point-in-polygon
    // algorithm
    bool pointInPolygon = false;
    for (unsigned int j = 0, k = subpath03.size()-1; j < subpath03.size();
        k = ++j)
    {
      if (((subpath03[j].Y() > v.Y()) != (subpath03[k].Y() > v.Y())) &&
          (v.X() < (subpath03[k].X()-subpath03[j].X()) *
           (v.Y()-subpath03[j].Y()) /
         (subpath03[k].Y()-subpath03[j].Y()) + subpath03[j].X()) )
      {
       pointInPolygon = !pointInPolygon;
      }
    }
    EXPECT_FALSE(pointInPolygon);

    // check extruded height
    EXPECT_TRUE((gz::math::equal(v.Z(), 0.0) ||
          gz::math::equal(v.Z(), 2.0)));
  }

  // verify same number of normals and vertices
  EXPECT_EQ(submesh->VertexCount(), submesh->NormalCount());

  // check normals
  for (unsigned int i = 0; i < submesh->NormalCount(); ++i)
  {
    gz::math::Vector3d v = submesh->Vertex(i);
    gz::math::Vector3d n = submesh->Normal(i);

    // vertex at 0 could be a bottom face or side face
    if (gz::math::equal(v.Z(), 0.0))
    {
      if (gz::math::equal(n.Z(), 0.0))
      {
        // side face - check non-zero normal
        EXPECT_TRUE(!(gz::math::equal(n.X(), 0.0) &&
                      gz::math::equal(n.Y(), 0.0)));
      }
      else
      {
        // bottom face - normal in -z direction
        EXPECT_TRUE((n == -gz::math::Vector3d::UnitZ) ||
                    (gz::math::equal(n.Z(), 0.0)));
      }
    }

    // vertex at height could be a top face or side face
    if (gz::math::equal(v.Z(), 10.0))
    {
      if (gz::math::equal(n.Z(), 0.0))
      {
        // side face - check non-zero normal
        EXPECT_TRUE(!(gz::math::equal(n.X(), 0.0) &&
                      gz::math::equal(n.Y(), 0.0)));
      }
      else
      {
        // top face - normal in +z direction
        EXPECT_TRUE((n == gz::math::Vector3d::UnitZ) ||
                    (gz::math::equal(n.Z(), 0.0)));
      }
    }
  }
}

/////////////////////////////////////////////////
TEST_P(MeshManager, CreateExtrudedPolylineInvalid)
{
  // test extruding invalid polyline
  std::vector<std::vector<gz::math::Vector2d> > path;
  std::vector<gz::math::Vector2d> subpath01;
  subpath01.emplace_back(0, 0);
  subpath01.emplace_back(0, 1);
  subpath01.emplace_back(0, 2);

  path.push_back(subpath01);

  std::string meshName = "extruded_path_invalid";
  double height = 10.0;
  common::MeshManager::Instance()->CreateExtrudedPolyline(
      meshName, path, height);

  // check mesh does not exist due to extrusion failure
  EXPECT_TRUE(!common::MeshManager::Instance()->HasMesh(meshName));
}

/////////////////////////////////////////////////
TEST_P(MeshManager, Remove)
{
  auto *mgr = common::MeshManager::Instance();

  EXPECT_FALSE(mgr->HasMesh("box"));
  mgr->CreateBox("box",
      gz::math::Vector3d(1, 1, 1),
      gz::math::Vector2d(0, 0));
  EXPECT_TRUE(mgr->HasMesh("box"));

  mgr->CreateSphere("sphere", 1.0, 1, 1);
  EXPECT_TRUE(mgr->HasMesh("sphere"));

  EXPECT_TRUE(mgr->RemoveMesh("box"));
  EXPECT_FALSE(mgr->HasMesh("box"));
  EXPECT_TRUE(mgr->HasMesh("sphere"));

  mgr->RemoveAll();
  EXPECT_FALSE(mgr->HasMesh("sphere"));
}

/////////////////////////////////////////////////
TEST_P(MeshManager, ConvexDecomposition)
{
  auto *mgr = common::MeshManager::Instance();
  const common::Mesh *boxMesh = mgr->Load(
      common::testing::TestFile("data", "box.dae"));

  ASSERT_NE(nullptr, boxMesh);
  EXPECT_EQ(1u, boxMesh->SubMeshCount());

  std::size_t maxConvexHulls = 4;
  std::size_t resolution = 1000;
  auto submesh = boxMesh->SubMeshByIndex(0u).lock();
  auto decomposed = common::MeshManager::ConvexDecomposition(
      *submesh, maxConvexHulls, resolution);

  // Decomposing a box should just produce a box
  EXPECT_EQ(1u, decomposed.size());
  common::SubMesh &boxSubmesh = decomposed[0];
  // A convex hull of a box should contain exactly 8 vertices
  EXPECT_EQ(8u, boxSubmesh.VertexCount());
  EXPECT_EQ(8u, boxSubmesh.NormalCount());
  EXPECT_EQ(36u, boxSubmesh.IndexCount());

  const common::Mesh *drillMesh = mgr->Load(
      common::testing::TestFile("data", "cordless_drill",
      "meshes", "cordless_drill.dae"));
  ASSERT_NE(nullptr, drillMesh);
  EXPECT_EQ(1u, drillMesh->SubMeshCount());
  submesh = drillMesh->SubMeshByIndex(0u).lock();
  decomposed = common::MeshManager::ConvexDecomposition(
      *submesh, maxConvexHulls, resolution);

  // A drill should be decomposed into multiple submeshes
  EXPECT_LT(1u, decomposed.size());
  EXPECT_GE(maxConvexHulls, decomposed.size());
  // Check submeshes are not empty
  for (const auto &d : decomposed)
  {
    const common::SubMesh &drillSubmesh = d;
    EXPECT_LT(3u, drillSubmesh.VertexCount());
    EXPECT_EQ(drillSubmesh.VertexCount(), drillSubmesh.NormalCount());
    EXPECT_LT(3u, drillSubmesh.IndexCount());
  }
}

/////////////////////////////////////////////////
TEST_P(MeshManager, MergeSubMeshes)
{
  auto *mgr = common::MeshManager::Instance();
  const common::Mesh *mesh = mgr->Load(
      common::testing::TestFile("data",
        "multiple_texture_coordinates_triangle.dae"));
  ASSERT_NE(nullptr, mesh);
  EXPECT_EQ(2u, mesh->SubMeshCount());
  auto submesh = mesh->SubMeshByIndex(0u).lock();
  ASSERT_NE(nullptr, submesh);
  EXPECT_EQ(3u, submesh->VertexCount());
  EXPECT_EQ(3u, submesh->NormalCount());
  EXPECT_EQ(3u, submesh->IndexCount());
  EXPECT_EQ(2u, submesh->TexCoordSetCount());
  EXPECT_EQ(3u, submesh->TexCoordCountBySet(0));
  EXPECT_EQ(3u, submesh->TexCoordCountBySet(1));
  auto submeshB = mesh->SubMeshByIndex(1u).lock();
  ASSERT_NE(nullptr, submeshB);
  EXPECT_EQ(3u, submeshB->VertexCount());
  EXPECT_EQ(3u, submeshB->NormalCount());
  EXPECT_EQ(3u, submeshB->IndexCount());
  EXPECT_EQ(3u, submeshB->TexCoordSetCount());
  EXPECT_EQ(3u, submeshB->TexCoordCountBySet(0));
  EXPECT_EQ(3u, submeshB->TexCoordCountBySet(1));
  EXPECT_EQ(3u, submeshB->TexCoordCountBySet(2));

  // merge all submeshes into one
  auto merged = common::MeshManager::MergeSubMeshes(*mesh);
  ASSERT_NE(nullptr, merged);
  EXPECT_FALSE(merged->Name().empty());
  EXPECT_EQ(1u, merged->SubMeshCount());
  auto mergedSubmesh = merged->SubMeshByIndex(0u).lock();
  ASSERT_NE(nullptr, mergedSubmesh);
  EXPECT_FALSE(mergedSubmesh->Name().empty());

  // Verify vertice, normals, indice, and texcoord values in the
  // final merged submesh
  EXPECT_EQ(6u, mergedSubmesh->VertexCount());
  EXPECT_EQ(6u, mergedSubmesh->NormalCount());
  EXPECT_EQ(6u, mergedSubmesh->IndexCount());
  EXPECT_EQ(3u, mergedSubmesh->TexCoordSetCount());
  EXPECT_EQ(6u, mergedSubmesh->TexCoordCountBySet(0));
  EXPECT_EQ(6u, mergedSubmesh->TexCoordCountBySet(1));
  EXPECT_EQ(6u, mergedSubmesh->TexCoordCountBySet(2));

  EXPECT_EQ(math::Vector3d(0, 0, 0), mergedSubmesh->Vertex(0u));
  EXPECT_EQ(math::Vector3d(10, 0, 0), mergedSubmesh->Vertex(1u));
  EXPECT_EQ(math::Vector3d(10, 10, 0), mergedSubmesh->Vertex(2u));
  EXPECT_EQ(math::Vector3d(10, 0, 0), mergedSubmesh->Vertex(3u));
  EXPECT_EQ(math::Vector3d(20, 0, 0), mergedSubmesh->Vertex(4u));
  EXPECT_EQ(math::Vector3d(20, 10, 0), mergedSubmesh->Vertex(5u));

  EXPECT_EQ(math::Vector3d(0, 0, 1), mergedSubmesh->Normal(0u));
  EXPECT_EQ(math::Vector3d(0, 0, 1), mergedSubmesh->Normal(1u));
  EXPECT_EQ(math::Vector3d(0, 0, 1), mergedSubmesh->Normal(2u));
  EXPECT_EQ(math::Vector3d(0, 0, 1), mergedSubmesh->Normal(3u));
  EXPECT_EQ(math::Vector3d(0, 0, 1), mergedSubmesh->Normal(4u));
  EXPECT_EQ(math::Vector3d(0, 0, 1), mergedSubmesh->Normal(5u));

  EXPECT_EQ(0u, mergedSubmesh->Index(0u));
  EXPECT_EQ(1u, mergedSubmesh->Index(1u));
  EXPECT_EQ(2u, mergedSubmesh->Index(2u));
  EXPECT_EQ(3u, mergedSubmesh->Index(3u));
  EXPECT_EQ(4u, mergedSubmesh->Index(4u));
  EXPECT_EQ(5u, mergedSubmesh->Index(5u));

  EXPECT_EQ(math::Vector2d(0, 1), mergedSubmesh->TexCoordBySet(0u, 0u));
  EXPECT_EQ(math::Vector2d(0, 1), mergedSubmesh->TexCoordBySet(1u, 0u));
  EXPECT_EQ(math::Vector2d(0, 1), mergedSubmesh->TexCoordBySet(2u, 0u));
  EXPECT_EQ(math::Vector2d(0, 1), mergedSubmesh->TexCoordBySet(3u, 0u));
  EXPECT_EQ(math::Vector2d(0, 1), mergedSubmesh->TexCoordBySet(4u, 0u));
  EXPECT_EQ(math::Vector2d(0, 1), mergedSubmesh->TexCoordBySet(5u, 0u));

  EXPECT_EQ(math::Vector2d(0, 1), mergedSubmesh->TexCoordBySet(0u, 1u));
  EXPECT_EQ(math::Vector2d(0, 1), mergedSubmesh->TexCoordBySet(1u, 1u));
  EXPECT_EQ(math::Vector2d(0, 1), mergedSubmesh->TexCoordBySet(2u, 1u));
  EXPECT_EQ(math::Vector2d(0, 0.5), mergedSubmesh->TexCoordBySet(3u, 1u));
  EXPECT_EQ(math::Vector2d(0, 0.4), mergedSubmesh->TexCoordBySet(4u, 1u));
  EXPECT_EQ(math::Vector2d(0, 0.3), mergedSubmesh->TexCoordBySet(5u, 1u));

  EXPECT_EQ(math::Vector2d(0, 0), mergedSubmesh->TexCoordBySet(0u, 2u));
  EXPECT_EQ(math::Vector2d(0, 0), mergedSubmesh->TexCoordBySet(1u, 2u));
  EXPECT_EQ(math::Vector2d(0, 0), mergedSubmesh->TexCoordBySet(2u, 2u));
  EXPECT_EQ(math::Vector2d(0, 0.8), mergedSubmesh->TexCoordBySet(3u, 2u));
  EXPECT_EQ(math::Vector2d(0, 0.7), mergedSubmesh->TexCoordBySet(4u, 2u));
  EXPECT_EQ(math::Vector2d(0, 0.6), mergedSubmesh->TexCoordBySet(5u, 2u));
}

/////////////////////////////////////////////////
TEST_P(MeshManager, CreateMesh)
{
  auto *mgr = common::MeshManager::Instance();
  std::string meshName = "test_create_mesh";

  // Pre-condition
  EXPECT_FALSE(mgr->HasMesh(meshName));
  EXPECT_EQ(nullptr, mgr->MeshByName(meshName));

  // Create the mesh
  common::Mesh *mutableMesh = mgr->CreateMesh(meshName);
  ASSERT_NE(nullptr, mutableMesh);
  EXPECT_EQ(meshName, mutableMesh->Name());

  // Fetch the newly created mesh
  const common::Mesh *cachedMesh = mgr->MeshByName(meshName);
  ASSERT_NE(nullptr, cachedMesh);
  EXPECT_EQ(mutableMesh, cachedMesh);
  EXPECT_TRUE(mgr->HasMesh(meshName));

  // Attempt to create the same mesh again should return nullptr safely
  common::Mesh *duplicateMesh = mgr->CreateMesh(meshName);
  EXPECT_EQ(nullptr, duplicateMesh);

  // Verify original mesh is intact
  const common::Mesh *verifyMesh = mgr->MeshByName(meshName);
  EXPECT_NE(nullptr, verifyMesh);
  EXPECT_EQ(meshName, verifyMesh->Name());

  mgr->RemoveAll();
}

/////////////////////////////////////////////////
TEST_P(MeshManager, LoadBox)
{
  auto *mgr = common::MeshManager::Instance();
  std::string filename = common::testing::TestFile("data", "box.dae");
  const common::Mesh *mesh = mgr->Load(filename);
  ASSERT_NE(nullptr, mesh);
  EXPECT_EQ(filename, mesh->Name());
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
  mgr->RemoveAll();
}

/////////////////////////////////////////////////
TEST_P(MeshManager, ShareVertices)
{
  auto *mgr = common::MeshManager::Instance();
  std::string filename = common::testing::TestFile("data", "box.dae");
  const common::Mesh *mesh = mgr->Load(filename);

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
  mgr->RemoveAll();
}

/////////////////////////////////////////////////
TEST_P(MeshManager, LoadZeroCount)
{
  auto *mgr = common::MeshManager::Instance();
  const common::Mesh *mesh = mgr->Load(
      common::testing::TestFile("data", "zero_count.dae"));
  ASSERT_TRUE(mesh);
  common::Console::Root().RawLogger().flush();
  std::string log = LogContent();

  // Expect no errors about missing values
  EXPECT_EQ(log.find("Loading what we can..."), std::string::npos);
  EXPECT_EQ(log.find("Vertex source missing float_array"), std::string::npos);
  EXPECT_EQ(log.find("Normal source missing float_array"), std::string::npos);

  // Expect the logs to contain information
  if (!forceAssimpEnv)
  {
    EXPECT_NE(log.find("Triangle input has a count of zero"), std::string::npos);
    EXPECT_NE(log.find("Vertex source has a float_array with a count of zero"),
        std::string::npos);
    EXPECT_NE(log.find("Normal source has a float_array with a count of zero"),
        std::string::npos);
  }
  mgr->RemoveAll();
}

/////////////////////////////////////////////////
TEST_P(MeshManager, Material)
{
  auto *mgr = common::MeshManager::Instance();
  std::string filename = common::testing::TestFile("data", "box.dae");
  const common::Mesh *mesh = mgr->Load(filename);
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

  const common::Mesh *meshOpaque = mgr->Load(
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

  mgr->RemoveAll();
}

/////////////////////////////////////////////////
TEST_P(MeshManager, TexCoordSets)
{
  auto *mgr = common::MeshManager::Instance();
  // This triangle mesh has multiple uv sets and vertices separated by
  // line breaks
  const common::Mesh *mesh = mgr->Load(
      common::testing::TestFile("data",
        "multiple_texture_coordinates_triangle.dae"));
  ASSERT_TRUE(mesh);

  EXPECT_EQ(6u, mesh->VertexCount());
  EXPECT_EQ(6u, mesh->NormalCount());
  EXPECT_EQ(6u, mesh->IndexCount());
  EXPECT_EQ(6u, mesh->TexCoordCount());
  EXPECT_EQ(2u, mesh->SubMeshCount());
  if (forceAssimpEnv)
  {
    EXPECT_EQ(1u, mesh->MaterialCount());
  }
  else
  {
    EXPECT_EQ(0u, mesh->MaterialCount());
  }

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
  mgr->RemoveAll();
}

/////////////////////////////////////////////////
TEST_P(MeshManager, LoadBoxWithAnimationOutsideSkeleton)
{
  auto *mgr = common::MeshManager::Instance();
  const common::Mesh *mesh = mgr->Load(
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
  mgr->RemoveAll();
}

/////////////////////////////////////////////////
TEST_P(MeshManager, LoadBoxInstControllerWithoutSkeleton)
{
  auto *mgr = common::MeshManager::Instance();
  const common::Mesh *mesh = mgr->Load(
      common::testing::TestFile("data",
        "box_inst_controller_without_skeleton.dae"));

  if (forceAssimpEnv)
  {
    EXPECT_EQ(24u, mesh->VertexCount());
    EXPECT_EQ(24u, mesh->TexCoordCount());
  }
  else
  {
    EXPECT_EQ(35u, mesh->VertexCount());
    EXPECT_EQ(35u, mesh->TexCoordCount());
  }
  EXPECT_EQ(36u, mesh->IndexCount());
  EXPECT_EQ(1u, mesh->SubMeshCount());
  EXPECT_EQ(1u, mesh->MaterialCount());
  common::SkeletonPtr skeleton = mesh->MeshSkeleton();
  EXPECT_LT(0u, skeleton->NodeCount());
  EXPECT_NE(nullptr, skeleton->NodeById("Armature_Bone"));
  mgr->RemoveAll();
}

/////////////////////////////////////////////////
TEST_P(MeshManager, LoadBoxMultipleInstControllers)
{
  auto *mgr = common::MeshManager::Instance();
  const common::Mesh *mesh = mgr->Load(
      common::testing::TestFile("data", "box_multiple_inst_controllers.dae"));

  if (forceAssimpEnv)
  {
    EXPECT_EQ(48u, mesh->VertexCount());
    EXPECT_EQ(48u, mesh->TexCoordCount());
  }
  else
  {
    EXPECT_EQ(70u, mesh->VertexCount());
    EXPECT_EQ(70u, mesh->TexCoordCount());
  }
  EXPECT_EQ(72u, mesh->IndexCount());
  EXPECT_EQ(2u, mesh->SubMeshCount());
  EXPECT_EQ(1u, mesh->MaterialCount());

  std::shared_ptr<common::SubMesh> submesh = mesh->SubMeshByIndex(0).lock();
  std::shared_ptr<common::SubMesh> submesh2 = mesh->SubMeshByIndex(1).lock();
  EXPECT_EQ(36u, submesh->IndexCount());
  EXPECT_EQ(36u, submesh2->IndexCount());
  if (forceAssimpEnv)
  {
    EXPECT_EQ(24u, submesh->VertexCount());
    EXPECT_EQ(24u, submesh2->VertexCount());
    EXPECT_EQ(24u, submesh->TexCoordCount());
    EXPECT_EQ(24u, submesh2->TexCoordCount());
  }
  else
  {
    EXPECT_EQ(35u, submesh->VertexCount());
    EXPECT_EQ(35u, submesh2->VertexCount());
    EXPECT_EQ(35u, submesh->TexCoordCount());
    EXPECT_EQ(35u, submesh2->TexCoordCount());
  }

  common::SkeletonPtr skeleton = mesh->MeshSkeleton();
  EXPECT_NE(nullptr, skeleton->NodeById("Armature_Bone"));
  if (!forceAssimpEnv)
  {
    EXPECT_NE(nullptr, skeleton->NodeById("Armature_Bone2"));
  }
  mgr->RemoveAll();
}

/////////////////////////////////////////////////
TEST_P(MeshManager, LoadBoxNestedAnimation)
{
  auto *mgr = common::MeshManager::Instance();
  const common::Mesh *mesh = mgr->Load(
      common::testing::TestFile("data", "box_nested_animation.dae"));

  if (forceAssimpEnv)
  {
    EXPECT_EQ(24u, mesh->VertexCount()); //
    EXPECT_EQ(24u, mesh->TexCoordCount()); //
  }
  else
  {
    EXPECT_EQ(35u, mesh->VertexCount()); //
    EXPECT_EQ(35u, mesh->TexCoordCount()); //
  }
  EXPECT_EQ(36u, mesh->IndexCount());
  EXPECT_EQ(1u, mesh->SubMeshCount());
  EXPECT_EQ(1u, mesh->MaterialCount());
  std::string nodeName;
  if (forceAssimpEnv)
  {
    nodeName = "Armature_Bone";
  }
  else
  {
    nodeName = "Bone";
  }
  common::SkeletonPtr skeleton = mesh->MeshSkeleton();
  ASSERT_EQ(1u, mesh->MeshSkeleton()->AnimationCount());
  common::SkeletonAnimation *anim = skeleton->Animation(0);
  EXPECT_EQ(anim->Name(), "Armature");
  EXPECT_EQ(1u, anim->NodeCount());
  EXPECT_TRUE(anim->HasNode(nodeName));
  auto nodeAnimation = anim->NodeAnimationByName(nodeName);
  EXPECT_NE(nullptr, nodeAnimation);
  EXPECT_EQ(nodeName, nodeAnimation->Name());
  auto poseStart = anim->PoseAt(0);
  math::Matrix4d expectedTrans = math::Matrix4d(
      1, 0, 0, 1,
      0, 1, 0, -1,
      0, 0, 1, 0,
      0, 0, 0, 1);
  EXPECT_EQ(expectedTrans, poseStart.at(nodeName));
  auto poseEnd = anim->PoseAt(1.666666);
  expectedTrans = math::Matrix4d(
        1, 0, 0, 2,
        0, 1, 0, -1,
        0, 0, 1, 0,
        0, 0, 0, 1);
  EXPECT_EQ(expectedTrans, poseEnd.at(nodeName));
  mgr->RemoveAll();
}

/////////////////////////////////////////////////
TEST_P(MeshManager, LoadBoxWithDefaultStride)
{
  auto *mgr = common::MeshManager::Instance();
  const common::Mesh *mesh = mgr->Load(
      common::testing::TestFile("data", "box_with_default_stride.dae"));
  
  if (forceAssimpEnv)
  {
    EXPECT_EQ(24u, mesh->VertexCount());
    EXPECT_EQ(24u, mesh->TexCoordCount());
  }
  else
  {
    EXPECT_EQ(35u, mesh->VertexCount());
    EXPECT_EQ(35u, mesh->TexCoordCount());
  }
  EXPECT_EQ(36u, mesh->IndexCount());
  EXPECT_EQ(1u, mesh->SubMeshCount());
  EXPECT_EQ(1u, mesh->MaterialCount());
  if (forceAssimpEnv)
  {
    ASSERT_EQ(0u, mesh->MeshSkeleton()->AnimationCount());
  }
  else
  {
    ASSERT_EQ(1u, mesh->MeshSkeleton()->AnimationCount());
  }
  mgr->RemoveAll();
}

/////////////////////////////////////////////////
TEST_P(MeshManager, LoadBoxWithMultipleGeoms)
{
  auto *mgr = common::MeshManager::Instance();
  const common::Mesh *mesh = mgr->Load(
      common::testing::TestFile("data", "box_with_multiple_geoms.dae"));

  EXPECT_EQ(72u, mesh->IndexCount());
  EXPECT_EQ(48u, mesh->VertexCount());
  if (forceAssimpEnv)
  {
    EXPECT_EQ(1u, mesh->MaterialCount());
  }
  else
  {
    EXPECT_EQ(0u, mesh->MaterialCount());
  }
  EXPECT_EQ(48u, mesh->TexCoordCount());
  ASSERT_EQ(1u, mesh->MeshSkeleton()->AnimationCount());
  ASSERT_EQ(2u, mesh->SubMeshCount());
  EXPECT_EQ(24u, mesh->SubMeshByIndex(0).lock()->NodeAssignmentsCount());
  EXPECT_EQ(0u, mesh->SubMeshByIndex(1).lock()->NodeAssignmentsCount());
  mgr->RemoveAll();
}

/////////////////////////////////////////////////
TEST_P(MeshManager, LoadBoxWithHierarchicalNodes)
{
  auto *mgr = common::MeshManager::Instance();
  const common::Mesh *mesh = mgr->Load(
      common::testing::TestFile("data", "box_with_hierarchical_nodes.dae"));
  ASSERT_EQ(5u, mesh->SubMeshCount());

  // node by itself
  EXPECT_EQ("StaticCube", mesh->SubMeshByIndex(0).lock()->Name());

  // nested node with no name so it takes the parent's name instead
  EXPECT_EQ("StaticCubeParent", mesh->SubMeshByIndex(1).lock()->Name());

  if (forceAssimpEnv)
  {
    // parent node containing child node with no name
    EXPECT_EQ("StaticCubeNestedNoName", mesh->SubMeshByIndex(2).lock()->Name());
  
    // nested node with name
    EXPECT_EQ("StaticCubeParent2", mesh->SubMeshByIndex(3).lock()->Name());
  
    // Parent of nested node with name
    EXPECT_EQ("StaticCubeNested", mesh->SubMeshByIndex(4).lock()->Name());
  }
  else
  {
    // parent node containing child node with no name
    EXPECT_EQ("StaticCubeParent", mesh->SubMeshByIndex(2).lock()->Name());
  
    // nested node with name
    EXPECT_EQ("StaticCubeNested", mesh->SubMeshByIndex(3).lock()->Name());
  
    // Parent of nested node with name
    EXPECT_EQ("StaticCubeParent2", mesh->SubMeshByIndex(4).lock()->Name());
  }
  mgr->RemoveAll();
}

/////////////////////////////////////////////////
TEST_P(MeshManager, MergeBoxWithDoubleSkeleton)
{
  auto *mgr = common::MeshManager::Instance();
  const common::Mesh *mesh = mgr->Load(
      common::testing::TestFile("data", "box_with_double_skeleton.dae"));
  std::string skeletonRootName;
  if (forceAssimpEnv)
  {
    skeletonRootName = "Scene";
  }
  else
  {
    skeletonRootName = "Armature";
  }
  EXPECT_TRUE(mesh->HasSkeleton());
  auto skeleton_ptr = mesh->MeshSkeleton();
  // The two skeletons have been joined and their root is the
  // animation root, called Armature (ColladaLoader) or Scene (AssimpLoader)
  EXPECT_EQ(skeleton_ptr->RootNode()->Name(), std::string(skeletonRootName));
  mgr->RemoveAll();
}

/////////////////////////////////////////////////
TEST_P(MeshManager, LoadCylinderAnimatedFrom3dsMax)
{
  // TODO(anyone) This test shows that the mesh loads without crashing, but the
  // mesh animation looks deformed when loaded. That still needs to be
  // addressed.
  auto *mgr = common::MeshManager::Instance();
  std::string filename = common::testing::TestFile("data",
        "cylinder_animated_from_3ds_max.dae");
  const common::Mesh *mesh = mgr->Load(filename);
  EXPECT_EQ(filename, mesh->Name());
  if (forceAssimpEnv)
  {
    EXPECT_EQ(194u, mesh->VertexCount());
    EXPECT_EQ(194u, mesh->NormalCount());
    EXPECT_EQ(1u, mesh->MaterialCount());
  }
  else
  {
    EXPECT_EQ(202u, mesh->VertexCount());
    EXPECT_EQ(202u, mesh->NormalCount());
    EXPECT_EQ(0u, mesh->MaterialCount());
  }
  EXPECT_EQ(852u, mesh->IndexCount());
  EXPECT_LT(0u, mesh->TexCoordCount());
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
  mgr->RemoveAll();
}

/////////////////////////////////////////////////
// Load animation without a name
TEST_P(MeshManager, NoAnimName)
{
  auto *mgr = common::MeshManager::Instance();
  std::string meshFilename =
    common::testing::TestFile("data", "box_with_no_animation_name.dae");

  const common::Mesh *mesh = mgr->Load(meshFilename);
  common::SkeletonPtr skeleton = mesh->MeshSkeleton();
  ASSERT_EQ(1u, skeleton->AnimationCount());
  common::SkeletonAnimation *anim = skeleton->Animation(0);
  auto animName = anim->Name();
  EXPECT_EQ(animName, "animation1");
  mgr->RemoveAll();
}

/////////////////////////////////////////////////
TEST_P(MeshManager, LoadObjBox)
{
  auto *mgr = common::MeshManager::Instance();
  std::string meshFilename = common::testing::TestFile("data", "box.obj");
  const common::Mesh *mesh = mgr->Load(meshFilename);

  EXPECT_EQ(meshFilename, mesh->Name());
  EXPECT_EQ(gz::math::Vector3d(1, 1, 1), mesh->Max());
  EXPECT_EQ(gz::math::Vector3d(-1, -1, -1), mesh->Min());
  // 36 vertices

  EXPECT_EQ(36u, mesh->IndexCount());
  EXPECT_EQ(0u, mesh->TexCoordCount());
  EXPECT_EQ(1u, mesh->SubMeshCount());
  EXPECT_EQ(1u, mesh->MaterialCount());
  EXPECT_EQ(mesh->VertexCount(), mesh->NormalCount());

  auto sm = mesh->SubMeshByIndex(0u);
  auto subMesh = sm.lock();
  std::vector<std::vector<double>> vertexValues {
    {1, -1, -1}, {1, -1, 1}, {-1, -1, 1},
    {1, -1, -1}, {-1, -1, 1}, {-1, -1, -1},
    {1, 1, -1}, {-1, 1, -1}, {-1, 1, 1},
    {1, 1, -1}, {-1, 1, 1}, {0.999999, 1, 1},
    {1, -1, -1}, {1, 1, -1}, {0.999999, 1, 1},
    {1, -1, -1}, {0.999999, 1, 1}, {1, -1, 1},
    {1, -1, 1}, {0.999999, 1, 1}, {-1, 1, 1},
    {1, -1, 1}, {-1, 1, 1}, {-1, -1, 1},
    {-1, -1, 1}, {-1, 1, 1}, {-1, 1, -1},
    {-1, -1, 1}, {-1, 1, -1}, {-1, -1, -1},
    {1, 1, -1}, {1, -1, -1}, {-1, -1, -1},
    {1, 1, -1}, {-1, -1, -1}, {-1, 1, -1}
  };
  
  EXPECT_EQ(mesh->IndexCount(), vertexValues.size());
  for (unsigned int i = 0; i < mesh->IndexCount(); ++i)
  {
    math::Vector3d vectorCoord(vertexValues[i][0], vertexValues[i][1], vertexValues[i][2]);
    EXPECT_EQ(subMesh->Vertex(subMesh->Index(i)), vectorCoord);
  }

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

  mgr->RemoveAll();
}

/////////////////////////////////////////////////
// This tests opening an OBJ file that has an invalid material reference
TEST_P(MeshManager, ObjInvalidMaterial)
{
  auto *mgr = common::MeshManager::Instance();

  std::string meshFilename =
    common::testing::TestFile("data", "invalid_material.obj");

  const common::Mesh *mesh = mgr->Load(meshFilename);

  EXPECT_TRUE(mesh != nullptr);
  mgr->RemoveAll();
}

/////////////////////////////////////////////////
// This tests opening an OBJ file that has PBR fields
TEST_P(MeshManager, PBR)
{
  auto *mgr = common::MeshManager::Instance();

  // load obj file exported by 3ds max that has pbr extension
  {
    std::string meshFilename =
      common::testing::TestFile("data", "cube_pbr.obj");

    const common::Mesh *mesh = mgr->Load(meshFilename);
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
    if (forceAssimpEnv)
    {
      EXPECT_EQ(
        common::testing::TestFile("data", "LightDome_Metalness.png_Metalness"), 
        pbr->MetalnessMap());
      EXPECT_EQ(
        common::testing::TestFile("data", "LightDome_Roughness.png_Roughness"), 
        pbr->RoughnessMap());
      EXPECT_EQ(
        common::testing::TestFile("data", "LightDome_Normal.png_Normal"), 
        pbr->NormalMap());
    }
    else
    {
      EXPECT_EQ("LightDome_Metalness.png", pbr->MetalnessMap());
      EXPECT_EQ("LightDome_Roughness.png", pbr->RoughnessMap());
      EXPECT_EQ("LightDome_Normal.png", pbr->NormalMap());
    }
    mgr->RemoveAll();
  }

  // load obj file exported by blender - it shoves pbr maps into
  // existing fields
  {
    std::string meshFilename =
      common::testing::TestFile("data", "blender_pbr.obj");

    const common::Mesh *mesh = mgr->Load(meshFilename);
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
    if (forceAssimpEnv)
    {
      EXPECT_DOUBLE_EQ(0.5, pbr->Roughness());
      // this fails
      // EXPECT_EQ("mesh_Metal.png", pbr->MetalnessMap());
      // EXPECT_EQ("mesh_Rough.png", pbr->RoughnessMap());
      EXPECT_EQ(common::testing::TestFile("data", "mesh_Normal.png_Normal"), 
        pbr->NormalMap());
    }
    else
    {
      EXPECT_DOUBLE_EQ(0, pbr->Roughness());
      EXPECT_EQ("mesh_Metal.png", pbr->MetalnessMap());
      EXPECT_EQ("mesh_Rough.png", pbr->RoughnessMap());
      EXPECT_EQ("mesh_Normal.png", pbr->NormalMap());
    }
    EXPECT_DOUBLE_EQ(0, pbr->Metalness());
    mgr->RemoveAll();
  }
}

/////////////////////////////////////////////////
TEST_P(MeshManager, LoadSTL)
{
  auto *mgr = common::MeshManager::Instance();
  auto mesh = mgr->Load("");
  EXPECT_EQ(nullptr, mesh);

  std::string cubeFilepath = common::testing::TestFile("data", "cube.stl");
  mesh = mgr->Load(cubeFilepath);
  EXPECT_NE(nullptr, mesh);

  EXPECT_EQ(cubeFilepath, mesh->Name().c_str());
  EXPECT_EQ(math::Vector3d(20, 0, 20), mesh->Max());
  EXPECT_EQ(math::Vector3d(0, -20, 0), mesh->Min());
  if (forceAssimpEnv)
  {
    EXPECT_EQ(1u, mesh->MaterialCount());
  }
  else
  {
    EXPECT_EQ(0u, mesh->MaterialCount());
  }
  
  EXPECT_EQ(36u, mesh->IndexCount());
  EXPECT_EQ(0u, mesh->TexCoordCount());
  EXPECT_EQ(1u, mesh->SubMeshCount());

  auto sm = mesh->SubMeshByIndex(0u);
  auto subMesh = sm.lock();
  EXPECT_NE(nullptr, subMesh);
  EXPECT_EQ(math::Vector3d(0, 0, -1), subMesh->Normal(0u));
  EXPECT_EQ(math::Vector3d(0, 0, -1), subMesh->Normal(1u));
  EXPECT_EQ(math::Vector3d(0, 0, -1), subMesh->Normal(2u));
  
  EXPECT_EQ(mesh->VertexCount(), mesh->NormalCount());
  std::vector<std::vector<double>> vertexValues {
    {20, 0, 0}, {0, -20, 0}, {0, 0, 0},
    {0, -20, 0}, {20, 0, 0}, {20, -20, 0},
    {20, -20, 20}, {0, -20, 0}, {20, -20, 0},
    {0, -20, 0}, {20, -20, 20}, {0, -20, 20},
    {20, 0, 0}, {20, -20, 20}, {20, -20, 0},
    {20, -20, 20}, {20, 0, 0}, {20, 0, 20},
    {20, -20, 20}, {0, 0, 20}, {0, -20, 20},
    {0, 0, 20}, {20, -20, 20}, {20, 0, 20},
    {0, 0, 20}, {0, -20, 0}, {0, -20, 20},
    {0, -20, 0}, {0, 0, 20}, {0, 0, 0},
    {0, 0, 20}, {20, 0, 0}, {0, 0, 0},
    {20, 0, 0}, {0, 0, 20}, {20, 0, 20}
  };

  EXPECT_EQ(vertexValues.size(), subMesh->IndexCount());
  for (unsigned int i = 0; i < subMesh->IndexCount(); ++i)
  {
    math::Vector3d vertexCoord(vertexValues[i][0], vertexValues[i][1], vertexValues[i][2]);
    EXPECT_EQ(subMesh->Vertex(subMesh->Index(i)), vertexCoord);
  }

  if (forceAssimpEnv)
  {
    EXPECT_STREQ("model", mesh->SubMeshByIndex(0).lock()->Name().c_str());
  }
  else
  {
    EXPECT_STREQ("", mesh->SubMeshByIndex(0).lock()->Name().c_str());
  }
  mgr->RemoveAll();

  std::string cubeBinFilepath = common::testing::TestFile("data", "cube_binary.stl");
  mesh = mgr->Load(cubeBinFilepath);
  EXPECT_NE(nullptr, mesh);

  EXPECT_EQ(cubeBinFilepath, mesh->Name().c_str());
  EXPECT_EQ(math::Vector3d(20, 0, 20), mesh->Max());
  EXPECT_EQ(math::Vector3d(0, -20, 0), mesh->Min());
  if (forceAssimpEnv)
  {
    EXPECT_EQ(1u, mesh->MaterialCount());
  }
  else
  {
    EXPECT_EQ(0u, mesh->MaterialCount());
  }
  EXPECT_EQ(36u, mesh->IndexCount());
  EXPECT_EQ(0u, mesh->TexCoordCount());
  EXPECT_EQ(1u, mesh->SubMeshCount());

  sm = mesh->SubMeshByIndex(0u);
  subMesh = sm.lock();
  EXPECT_NE(nullptr, subMesh);
  EXPECT_EQ(math::Vector3d(0, 0, -1), subMesh->Normal(0u));
  EXPECT_EQ(math::Vector3d(0, 0, -1), subMesh->Normal(1u));
  EXPECT_EQ(math::Vector3d(0, 0, -1), subMesh->Normal(2u));
  EXPECT_EQ(mesh->VertexCount(), mesh->NormalCount());
  // Coordinates of the vertices should be the same as in the cube.stl
  EXPECT_EQ(vertexValues.size(), subMesh->IndexCount());
  for (unsigned int i = 0; i < subMesh->IndexCount(); ++i)
  {
    math::Vector3d vertexCoord(vertexValues[i][0], vertexValues[i][1], vertexValues[i][2]);
    EXPECT_EQ(subMesh->Vertex(subMesh->Index(i)), vertexCoord);
  }

  EXPECT_STREQ("", mesh->SubMeshByIndex(0).lock()->Name().c_str());
  mgr->RemoveAll();
}

/////////////////////////////////////////////////
// Open a non existing file
TEST_P(MeshManager, NonExistingMesh)
{
  auto *mgr = common::MeshManager::Instance();
  std::string meshFilename =
    common::testing::TestFile("data", "non_existing_mesh.glb");
  const common::Mesh *mesh = mgr->Load(meshFilename);

  EXPECT_EQ(mesh, nullptr);
  mgr->RemoveAll();
}

/////////////////////////////////////////////////
// This test opens a FBX file
TEST_P(MeshManager, LoadFbxBox)
{
  auto *mgr = common::MeshManager::Instance();
  std::string meshFilename =
    common::testing::TestFile("data", "box.fbx");
  const common::Mesh *mesh = mgr->Load(meshFilename);

  EXPECT_EQ(meshFilename, mesh->Name());
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
  mgr->RemoveAll();
}

/////////////////////////////////////////////////
// This test opens a GLB file
TEST_P(MeshManager, LoadGlTF2Box)
{
  auto *mgr = common::MeshManager::Instance();
  std::string meshFilename =
    common::testing::TestFile("data", "box.glb");
  const common::Mesh *mesh = mgr->Load(meshFilename);

  EXPECT_EQ(meshFilename, mesh->Name());
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
  mgr->RemoveAll();
}

/////////////////////////////////////////////////
// Open a gltf mesh with an external texture
TEST_P(MeshManager, LoadGlTF2BoxExternalTexture)
{
  auto *mgr = common::MeshManager::Instance();
  std::string meshFilename =
    common::testing::TestFile("data", "gltf", "PurpleCube.gltf");
  const common::Mesh *mesh = mgr->Load(meshFilename);

  EXPECT_EQ(meshFilename, mesh->Name());

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
  mgr->RemoveAll();
}

/////////////////////////////////////////////////
// Open a gltf mesh with transmission extension
TEST_P(MeshManager, LoadGlTF2BoxTransmission)
{
  auto *mgr = common::MeshManager::Instance();
  std::string meshFilename =
    common::testing::TestFile("data", "box_transmission.glb");
  const common::Mesh *mesh = mgr->Load(meshFilename);

  EXPECT_EQ(meshFilename, mesh->Name());

  // Make sure we can read the submesh name
  EXPECT_STREQ("Cube", mesh->SubMeshByIndex(0).lock()->Name().c_str());

  EXPECT_EQ(mesh->MaterialCount(), 1u);

  const common::MaterialPtr mat = mesh->MaterialByIndex(0u);
  ASSERT_TRUE(mat.get());
  // transmission currently modeled as transparency
  EXPECT_FLOAT_EQ(0.1f, mat->Transparency());
  mgr->RemoveAll();
}

/////////////////////////////////////////////////
// This test loads a box glb mesh with embedded compressed jpeg texture
TEST_P(MeshManager, LoadGlTF2BoxWithJPEGTexture)
{
  auto *mgr = common::MeshManager::Instance();
  std::string meshFilename =
    common::testing::TestFile("data", "box_texture_jpg.glb");
  const common::Mesh *mesh = mgr->Load(meshFilename);

  EXPECT_EQ(meshFilename, mesh->Name());
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
  mgr->RemoveAll();
}

/////////////////////////////////////////////////
// Use a fully featured glb test asset, including PBR textures, emissive maps
// embedded textures, lightmaps, animations to test advanced glb features
TEST_P(MeshManager, LoadGlbPbrAsset)
{
  auto *mgr = common::MeshManager::Instance();
  std::string meshFilename =
    common::testing::TestFile("data", "fully_featured.glb");
  const common::Mesh *mesh = mgr->Load(meshFilename);

  EXPECT_EQ(meshFilename, mesh->Name());

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
  mgr->RemoveAll();
}

/////////////////////////////////////////////////
// Checks for null root node animation and valid
// x displacement in non root node's animation.
TEST_P(MeshManager, CheckNonRootDisplacement)
{
  auto *mgr = common::MeshManager::Instance();
  const common::Mesh *mesh = mgr->Load(
    common::testing::TestFile("data", "walk.dae"));
  auto meshSkel =  mesh->MeshSkeleton();
  std::string rootNodeName = meshSkel->RootNode()->Name();
  common::SkeletonAnimation *skelAnim = meshSkel->Animation(0);
  common::NodeAnimation *rootNode = skelAnim->NodeAnimationByName(rootNodeName);
  // EXPECT_EQ(nullptr, rootNode); // this fails when GZ_MESH_FORCE_ASSIMP=false
  auto xDisplacement = skelAnim->XDisplacement();
  ASSERT_TRUE(xDisplacement);
  mgr->RemoveAll();
}

/////////////////////////////////////////////////
TEST_P(MeshManager, LoadGLTF2Triangle)
{
  auto *mgr = common::MeshManager::Instance();
  const common::Mesh *mesh = mgr->Load(
      common::testing::TestFile("data",
        "multiple_texture_coordinates_triangle.glb"));
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
  mgr->RemoveAll();
}

#endif
