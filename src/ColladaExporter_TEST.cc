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
#include "tinyxml2.h"

#include "test_config.h"
#include "ignition/common/Mesh.hh"
#include "ignition/common/SubMesh.hh"
#include "ignition/common/ColladaLoader.hh"
#include "ignition/common/ColladaExporter.hh"
#include "test/util.hh"

#ifdef _WIN32
  #define snprintf _snprintf
#endif

using namespace ignition;

class ColladaExporter : public ignition::testing::AutoLogFixture { };

/////////////////////////////////////////////////
TEST_F(ColladaExporter, ExportBox)
{
  std::string filenameIn = std::string(PROJECT_SOURCE_PATH) +
      "/test/data/box.dae";

  std::string pathOut = common::cwd();
  common::createDirectories(pathOut + "/" + "tmp");
  std::string filenameOut = pathOut+ "/tmp/box_exported";

  common::ColladaLoader loader;
  const common::Mesh *meshOriginal = loader.Load(
      filenameIn);

  // Export with extension
  common::ColladaExporter exporter;
  exporter.Export(meshOriginal, filenameOut, false);

  // Check .dae file
  tinyxml2::XMLDocument xmlDoc;
  filenameOut = filenameOut + ".dae";
  EXPECT_EQ(xmlDoc.LoadFile(filenameOut.c_str()), tinyxml2::XML_SUCCESS);
  ASSERT_TRUE(xmlDoc.FirstChildElement("COLLADA") != nullptr);

  const char *countDae = xmlDoc.FirstChildElement("COLLADA")
                               ->FirstChildElement("library_geometries")
                               ->FirstChildElement("geometry")
                               ->FirstChildElement("mesh")
                               ->FirstChildElement("source")
                               ->FirstChildElement("float_array")
                               ->Attribute("count");
  unsigned int countMeshInt =
    meshOriginal->SubMeshByIndex(0).lock()->VertexCount()*3;
  char countMesh[100];
  snprintf(countMesh, sizeof(countMesh), "%u", countMeshInt);

  EXPECT_STREQ(countDae, countMesh);

  // Reload mesh and compare
  const common::Mesh *meshReloaded = loader.Load(filenameOut);

  EXPECT_EQ(meshOriginal->Name(), meshReloaded->Name());
  EXPECT_EQ(meshOriginal->Max(), meshReloaded->Max());
  EXPECT_EQ(meshOriginal->Min(), meshReloaded->Min());
  EXPECT_EQ(meshOriginal->SubMeshCount(), meshReloaded->SubMeshCount());
  EXPECT_EQ(meshOriginal->MaterialCount(),
      meshReloaded->MaterialCount());
  EXPECT_EQ(meshOriginal->IndexCount(), meshReloaded->IndexCount());
  EXPECT_EQ(meshOriginal->VertexCount(), meshReloaded->VertexCount());
  EXPECT_EQ(meshOriginal->NormalCount(), meshReloaded->NormalCount());
  EXPECT_EQ(meshOriginal->TexCoordCount(),
      meshReloaded->TexCoordCount());
  for (unsigned int i = 0; i < meshOriginal->SubMeshCount(); ++i)
  {
    for (unsigned int j = 0; j < meshOriginal->VertexCount(); ++j)
    {
      EXPECT_EQ(meshOriginal->SubMeshByIndex(i).lock()->Vertex(j),
                meshReloaded->SubMeshByIndex(i).lock()->Vertex(j));
    }
    for (unsigned int j = 0; j < meshOriginal->NormalCount(); ++j)
    {
      EXPECT_EQ(meshOriginal->SubMeshByIndex(i).lock()->Normal(j),
                meshReloaded->SubMeshByIndex(i).lock()->Normal(j));
    }
    for (unsigned int j = 0; j < meshOriginal->TexCoordCount(); ++j)
    {
      EXPECT_EQ(meshOriginal->SubMeshByIndex(i).lock()->TexCoord(j),
                meshReloaded->SubMeshByIndex(i).lock()->TexCoord(j));
    }
  }

  // Remove temp directory
  common::removeAll(pathOut + "/tmp");
}

/////////////////////////////////////////////////
TEST_F(ColladaExporter, ExportCordlessDrill)
{
  common::ColladaLoader loader;
  const common::Mesh *meshOriginal = loader.Load(
      std::string(PROJECT_SOURCE_PATH) +
      "/test/data/cordless_drill/meshes/cordless_drill.dae");

  std::string pathOut = common::cwd();
  common::createDirectories(pathOut + "/" + "tmp");

  common::ColladaExporter exporter;
  exporter.Export(meshOriginal, pathOut + "/tmp/cordless_drill_exported", true);

  // Check .dae file
  tinyxml2::XMLDocument xmlDoc;
  std::string filename = pathOut +
    "/tmp/cordless_drill_exported/meshes/cordless_drill_exported.dae";

  EXPECT_EQ(xmlDoc.LoadFile(filename.c_str()), tinyxml2::XML_SUCCESS);

  ASSERT_TRUE(xmlDoc.FirstChildElement("COLLADA") != nullptr);
  ASSERT_TRUE(xmlDoc.FirstChildElement(
        "COLLADA")->FirstChildElement("library_geometries") != nullptr);

  tinyxml2::XMLElement *geometryXml = xmlDoc.FirstChildElement("COLLADA")
      ->FirstChildElement("library_geometries")
      ->FirstChildElement("geometry");
  ASSERT_TRUE(geometryXml != nullptr);

  for (unsigned int i = 0; i < meshOriginal->SubMeshCount(); ++i)
  {
    unsigned int countMeshInt =
      meshOriginal->SubMeshByIndex(i).lock()->VertexCount()*3;
    char countMesh[100];
    snprintf(countMesh, sizeof(countMesh), "%u", countMeshInt);

    const char *countDae = geometryXml
        ->FirstChildElement("mesh")
        ->FirstChildElement("source")
        ->FirstChildElement("float_array")
        ->Attribute("count");

    EXPECT_STREQ(countDae, countMesh);

    geometryXml = geometryXml->NextSiblingElement("geometry");
  }

  // Reload mesh and compare
  const common::Mesh *meshReloaded = loader.Load(pathOut +
      "/tmp/cordless_drill_exported/meshes/cordless_drill_exported.dae");

  EXPECT_EQ(meshOriginal->Name(), meshReloaded->Name());
  EXPECT_EQ(meshOriginal->Max(), meshReloaded->Max());
  EXPECT_EQ(meshOriginal->Min(), meshReloaded->Min());
  EXPECT_EQ(meshOriginal->SubMeshCount(), meshReloaded->SubMeshCount());
  EXPECT_EQ(meshOriginal->MaterialCount(),
      meshReloaded->MaterialCount());
  EXPECT_EQ(meshOriginal->IndexCount(), meshReloaded->IndexCount());
  EXPECT_EQ(meshOriginal->VertexCount(), meshReloaded->VertexCount());
  EXPECT_EQ(meshOriginal->NormalCount(), meshReloaded->NormalCount());
  EXPECT_EQ(meshOriginal->TexCoordCount(),
      meshReloaded->TexCoordCount());
  for (unsigned int i = 0; i < meshOriginal->SubMeshCount(); ++i)
  {
    for (unsigned int j = 0; j < meshOriginal->VertexCount(); ++j)
    {
      EXPECT_EQ(meshOriginal->SubMeshByIndex(i).lock()->Vertex(j),
          meshReloaded->SubMeshByIndex(i).lock()->Vertex(j));
    }
    for (unsigned int j = 0; j < meshOriginal->NormalCount(); ++j)
    {
      EXPECT_EQ(meshOriginal->SubMeshByIndex(i).lock()->Normal(j),
          meshReloaded->SubMeshByIndex(i).lock()->Normal(j));
    }
    for (unsigned int j = 0; j < meshOriginal->TexCoordCount(); ++j)
    {
      EXPECT_EQ(meshOriginal->SubMeshByIndex(i).lock()->TexCoord(j),
                meshReloaded->SubMeshByIndex(i).lock()->TexCoord(j));
    }
  }

  // Remove temp directory
  common::removeAll(pathOut + "/tmp");
}

/////////////////////////////////////////////////
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
