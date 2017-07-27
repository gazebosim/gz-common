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
#include "ignition/math/Vector3.hh"
#include "ignition/common/Mesh.hh"
#include "ignition/common/SubMesh.hh"
#include "test/util.hh"

using namespace ignition;

class SubMeshTest : public ignition::testing::AutoLogFixture { };

/////////////////////////////////////////////////
TEST_F(SubMeshTest, SubMesh)
{
  common::SubMeshPtr submesh(new common::SubMesh());
  EXPECT_TRUE(submesh != NULL);

  submesh->SetName("new_submesh");
  EXPECT_EQ(submesh->Name(), "new_submesh");

  submesh->SetPrimitiveType(common::SubMesh::TRIANGLES);
  EXPECT_EQ(submesh->SubMeshPrimitiveType(), common::SubMesh::TRIANGLES);

  submesh->SetMaterialIndex(3u);
  EXPECT_EQ(submesh->MaterialIndex(), 3u);

  // verify empty submesh
  EXPECT_EQ(submesh->Min(), math::Vector3d::Zero);
  EXPECT_EQ(submesh->Max(), math::Vector3d::Zero);
  EXPECT_EQ(submesh->VertexCount(), 0u);
  EXPECT_EQ(submesh->NormalCount(), 0u);
  EXPECT_EQ(submesh->TexCoordCount(), 0u);
  EXPECT_EQ(submesh->IndexCount(), 0u);
  EXPECT_EQ(submesh->NodeAssignmentsCount(), 0u);
  EXPECT_EQ(submesh->MaxIndex(), 0u);

  // create geometry test data
  math::Vector3d v0(0, 0, 1);
  math::Vector3d v1(2, 0, 0);
  math::Vector3d v2(0, 3, 3);
  math::Vector3d n0(1, 0, 0);
  math::Vector3d n1(0, 1, 0);
  math::Vector3d n2(0.7, 0.7, 0);
  math::Vector2d uv0(0, 0);
  math::Vector2d uv1(0, 1);
  math::Vector2d uv2(1, 0);

  // add vertices
  submesh->AddVertex(v0);
  EXPECT_EQ(submesh->VertexCount(), 1u);
  EXPECT_EQ(submesh->Vertex(0u), v0);
  EXPECT_TRUE(submesh->HasVertex(v0));
  EXPECT_EQ(submesh->IndexOfVertex(v0), 0);

  submesh->AddVertex(v1);
  EXPECT_EQ(submesh->VertexCount(), 2u);
  EXPECT_EQ(submesh->Vertex(1u), v1);
  EXPECT_TRUE(submesh->HasVertex(v1));
  EXPECT_EQ(submesh->IndexOfVertex(v1), 1);

  submesh->AddVertex(v2.X(), v2.Y(), v2.Z());
  EXPECT_EQ(submesh->VertexCount(), 3u);
  EXPECT_EQ(submesh->Vertex(2u), v2);
  EXPECT_TRUE(submesh->HasVertex(v2));
  EXPECT_EQ(submesh->IndexOfVertex(v2), 2);

  // max / min
  math::Vector3d max(2, 3, 3);
  math::Vector3d min(0, 0, 0);
  EXPECT_EQ(submesh->Max(), max);
  EXPECT_EQ(submesh->Min(), min);

  // add normals
  submesh->AddNormal(n0);
  EXPECT_EQ(submesh->NormalCount(), 1u);
  EXPECT_EQ(submesh->Normal(0u), n0);
  EXPECT_TRUE(submesh->HasNormal(0u));

  submesh->AddNormal(n1);
  EXPECT_EQ(submesh->NormalCount(), 2u);
  EXPECT_EQ(submesh->Normal(1u), n1);
  EXPECT_TRUE(submesh->HasNormal(1u));

  submesh->AddNormal(n2.X(), n2.Y(), n2.Z());
  EXPECT_EQ(submesh->NormalCount(), 3u);
  EXPECT_EQ(submesh->Normal(2u), n2);
  EXPECT_TRUE(submesh->HasNormal(2u));

  // add texcoord
  submesh->AddTexCoord(uv0);
  EXPECT_EQ(submesh->TexCoordCount(), 1u);
  EXPECT_EQ(submesh->TexCoord(0u), uv0);
  EXPECT_TRUE(submesh->HasTexCoord(0u));

  submesh->AddTexCoord(uv1);
  EXPECT_EQ(submesh->TexCoordCount(), 2u);
  EXPECT_EQ(submesh->TexCoord(1u), uv1);
  EXPECT_TRUE(submesh->HasTexCoord(1u));

  submesh->AddTexCoord(uv2.X(), uv2.Y());
  EXPECT_EQ(submesh->TexCoordCount(), 3u);
  EXPECT_EQ(submesh->TexCoord(2u), uv2);
  EXPECT_TRUE(submesh->HasTexCoord(2u));

  // add index
  submesh->AddIndex(0u);
  EXPECT_EQ(submesh->IndexCount(), 1u);
  EXPECT_EQ(submesh->Index(0), 0);

  submesh->AddIndex(2u);
  EXPECT_EQ(submesh->IndexCount(), 2u);
  EXPECT_EQ(submesh->Index(1u), 2);

  submesh->AddIndex(1u);
  EXPECT_EQ(submesh->IndexCount(), 3u);
  EXPECT_EQ(submesh->Index(2u), 1);

  // add node assignment
  submesh->AddNodeAssignment(1u, 0u, 0.5f);
  EXPECT_EQ(submesh->NodeAssignmentsCount(), 1u);

  common::NodeAssignment node = submesh->NodeAssignmentByIndex(0);
  EXPECT_EQ(node.vertexIndex, 1u);
  EXPECT_EQ(node.nodeIndex, 0u);
  EXPECT_DOUBLE_EQ(node.weight, 0.5f);

  common::NodeAssignment nodeInvalid = submesh->NodeAssignmentByIndex(1);
  EXPECT_EQ(nodeInvalid.vertexIndex, 0u);
  EXPECT_EQ(nodeInvalid.nodeIndex, 0u);
  EXPECT_DOUBLE_EQ(nodeInvalid.weight, 0.0f);

  // test directly setting values and failure cases
  submesh->SetVertex(2u, v0);
  EXPECT_EQ(submesh->Vertex(2u), v0);
  submesh->SetVertex(2u, v2);
  EXPECT_EQ(submesh->Vertex(2u), v2);
  submesh->SetVertex(3u, math::Vector3d(0.9, 2, 4));
  EXPECT_EQ(submesh->Vertex(3u), math::Vector3d::Zero);
  EXPECT_FALSE(submesh->HasVertex(math::Vector3d(0.9, 2, 4)));
  EXPECT_EQ(submesh->IndexOfVertex(math::Vector3d(0.9, 2, 4)), -1);

  submesh->SetNormal(2u, n0);
  EXPECT_EQ(submesh->Normal(2u), n0);
  submesh->SetNormal(2u, n2);
  EXPECT_EQ(submesh->Normal(2u), n2);
  submesh->SetNormal(3u, math::Vector3d(20, 3, 0.4));
  EXPECT_EQ(submesh->Normal(3u), math::Vector3d::Zero);
  EXPECT_FALSE(submesh->HasNormal(4u));

  submesh->SetTexCoord(2u, uv1);
  EXPECT_EQ(submesh->TexCoord(2u), uv1);
  submesh->SetTexCoord(2u, uv2);
  EXPECT_EQ(submesh->TexCoord(2u), uv2);
  submesh->SetTexCoord(3u, math::Vector2d(0.3, 0.4));
  EXPECT_EQ(submesh->TexCoord(3u), math::Vector2d(0, 0));

  submesh->SetIndex(2u, 9u);
  EXPECT_EQ(submesh->Index(2u), 9);
  submesh->SetIndex(2u, 2u);
  EXPECT_EQ(submesh->Index(2u), 2);
  submesh->SetIndex(3u, 999);
  EXPECT_EQ(submesh->Index(3u), -1);
  EXPECT_EQ(submesh->MaxIndex(), 2u);

  // scale
  math::Vector3d scale(1, 0.5, 2);
  submesh->Scale(scale);

  EXPECT_EQ(submesh->Vertex(0), v0 * scale);
  EXPECT_EQ(submesh->Vertex(1), v1 * scale);
  EXPECT_EQ(submesh->Vertex(2), v2 * scale);

  EXPECT_EQ(submesh->Normal(0), n0);
  EXPECT_EQ(submesh->Normal(1), n1);
  EXPECT_EQ(submesh->Normal(2), n2);

  EXPECT_EQ(submesh->TexCoord(0), uv0);
  EXPECT_EQ(submesh->TexCoord(1), uv1);
  EXPECT_EQ(submesh->TexCoord(2), uv2);

  math::Vector3d scaleInv(1.0 / scale.X(), 1.0 / scale.Y(), 1.0 / scale.Z());
  submesh->Scale(scaleInv);

  EXPECT_EQ(submesh->Vertex(0), v0);
  EXPECT_EQ(submesh->Vertex(1), v1);
  EXPECT_EQ(submesh->Vertex(2), v2);

  EXPECT_EQ(submesh->Normal(0), n0);
  EXPECT_EQ(submesh->Normal(1), n1);
  EXPECT_EQ(submesh->Normal(2), n2);

  EXPECT_EQ(submesh->TexCoord(0), uv0);
  EXPECT_EQ(submesh->TexCoord(1), uv1);
  EXPECT_EQ(submesh->TexCoord(2), uv2);

  // translate
  math::Vector3d t0(2, 3, 4);
  submesh->Translate(t0);

  EXPECT_EQ(submesh->Vertex(0), v0 + t0);
  EXPECT_EQ(submesh->Vertex(1), v1 + t0);
  EXPECT_EQ(submesh->Vertex(2), v2 + t0);

  EXPECT_EQ(submesh->Normal(0), n0);
  EXPECT_EQ(submesh->Normal(1), n1);
  EXPECT_EQ(submesh->Normal(2), n2);

  EXPECT_EQ(submesh->TexCoord(0), uv0);
  EXPECT_EQ(submesh->TexCoord(1), uv1);
  EXPECT_EQ(submesh->TexCoord(2), uv2);

  // center
  math::Vector3d c0(8, 20, 3);
  submesh->Center(c0);

  math::Vector3d t = c0 - (min + t0 + (max-min)/2);
  EXPECT_EQ(submesh->Vertex(0), v0 + t0 + t);
  EXPECT_EQ(submesh->Vertex(1), v1 + t0 + t);
  EXPECT_EQ(submesh->Vertex(2), v2 + t0 + t);

  // copy constructor
  common::SubMeshPtr submeshCopy(new common::SubMesh(*(submesh.get())));
  EXPECT_TRUE(submeshCopy != NULL);
  EXPECT_EQ(submeshCopy->Name(), submesh->Name());
  EXPECT_EQ(submeshCopy->MaterialIndex(), submesh->MaterialIndex());
  EXPECT_EQ(submeshCopy->SubMeshPrimitiveType(),
      submesh->SubMeshPrimitiveType());
  EXPECT_EQ(submeshCopy->VertexCount(), submesh->VertexCount());
  EXPECT_EQ(submeshCopy->NormalCount(), submesh->NormalCount());
  EXPECT_EQ(submeshCopy->TexCoordCount(), submesh->TexCoordCount());
  EXPECT_EQ(submeshCopy->IndexCount(), submesh->IndexCount());
  EXPECT_EQ(submeshCopy->NodeAssignmentsCount(),
      submesh->NodeAssignmentsCount());

  for (unsigned int i = 0; i < submeshCopy->VertexCount(); ++i)
    EXPECT_EQ(submeshCopy->Vertex(i), submesh->Vertex(i));
  for (unsigned int i = 0; i < submeshCopy->NormalCount(); ++i)
    EXPECT_EQ(submeshCopy->Normal(i), submesh->Normal(i));
  for (unsigned int i = 0; i < submeshCopy->TexCoordCount(); ++i)
    EXPECT_EQ(submeshCopy->TexCoord(i), submesh->TexCoord(i));
  for (unsigned int i = 0; i < submeshCopy->IndexCount(); ++i)
    EXPECT_EQ(submeshCopy->Index(i), submesh->Index(i));
  for (unsigned int i = 0; i < submeshCopy->NodeAssignmentsCount(); ++i)
  {
    common::NodeAssignment nodeCopy =
      submeshCopy->NodeAssignmentByIndex(i);
    EXPECT_EQ(nodeCopy.vertexIndex, 1u);
    EXPECT_EQ(nodeCopy.nodeIndex, 0u);
    EXPECT_DOUBLE_EQ(nodeCopy.weight, 0.5f);
  }

  // fill array
  double *vertices = NULL;
  int *indices = NULL;
  common::SubMeshPtr submeshEmpty(new common::SubMesh());
  submeshEmpty->FillArrays(&vertices, &indices);
  EXPECT_TRUE(vertices == NULL);
  EXPECT_TRUE(indices == NULL);

  vertices = new double[9];
  indices = new int[3];
  submesh->FillArrays(&vertices, &indices);
  for (unsigned int i = 0; i < submeshCopy->VertexCount(); ++i)
  {
    EXPECT_DOUBLE_EQ(vertices[i*3], submesh->Vertex(i).X());
    EXPECT_DOUBLE_EQ(vertices[i*3+1], submesh->Vertex(i).Y());
    EXPECT_DOUBLE_EQ(vertices[i*3+2], submesh->Vertex(i).Z());
  }
  for (unsigned int i = 0; i < submeshCopy->IndexCount(); ++i)
    EXPECT_EQ(indices[i], submesh->Index(i));


  // recalculate normal and verify they are different
  submesh->RecalculateNormals();
  for (unsigned int i = 0; i < submeshCopy->NormalCount(); ++i)
    EXPECT_NE(submeshCopy->Normal(i), submesh->Normal(i));
  for (unsigned int i = 0; i < submeshCopy->VertexCount(); ++i)
    EXPECT_EQ(submeshCopy->Vertex(i), submesh->Vertex(i));
  for (unsigned int i = 0; i < submeshCopy->TexCoordCount(); ++i)
    EXPECT_EQ(submeshCopy->TexCoord(i), submesh->TexCoord(i));
  for (unsigned int i = 0; i < submeshCopy->IndexCount(); ++i)
    EXPECT_EQ(submeshCopy->Index(i), submesh->Index(i));
  for (unsigned int i = 0; i < submeshCopy->NodeAssignmentsCount(); ++i)
  {
    common::NodeAssignment nodeCopy =
      submeshCopy->NodeAssignmentByIndex(i);
    EXPECT_EQ(nodeCopy.vertexIndex, 1u);
    EXPECT_EQ(nodeCopy.nodeIndex, 0u);
    EXPECT_DOUBLE_EQ(nodeCopy.weight, 0.5f);
  }
  submeshCopy->AddNormal(math::Vector3d(0, 1, 0));
  submeshCopy->RecalculateNormals();
  for (unsigned int i = 0; i < submeshCopy->NormalCount(); ++i)
    EXPECT_EQ(submeshCopy->Normal(i), submesh->Normal(i));
  for (unsigned int i = 0; i < submeshCopy->VertexCount(); ++i)
    EXPECT_EQ(submeshCopy->Vertex(i), submesh->Vertex(i));
  for (unsigned int i = 0; i < submeshCopy->TexCoordCount(); ++i)
    EXPECT_EQ(submeshCopy->TexCoord(i), submesh->TexCoord(i));
  for (unsigned int i = 0; i < submeshCopy->IndexCount(); ++i)
    EXPECT_EQ(submeshCopy->Index(i), submesh->Index(i));
  for (unsigned int i = 0; i < submeshCopy->NodeAssignmentsCount(); ++i)
  {
    common::NodeAssignment nodeCopy =
      submeshCopy->NodeAssignmentByIndex(i);
    EXPECT_EQ(nodeCopy.vertexIndex, 1u);
    EXPECT_EQ(nodeCopy.nodeIndex, 0u);
    EXPECT_DOUBLE_EQ(nodeCopy.weight, 0.5f);
  }

  // GenSphericalTexCoord and verify they are different
  submesh->GenSphericalTexCoord(math::Vector3d::Zero);
  for (unsigned int i = 0; i < submeshCopy->TexCoordCount(); ++i)
    EXPECT_NE(submeshCopy->TexCoord(i), submesh->TexCoord(i));
  for (unsigned int i = 0; i < submeshCopy->NormalCount(); ++i)
    EXPECT_EQ(submeshCopy->Normal(i), submesh->Normal(i));
  for (unsigned int i = 0; i < submeshCopy->VertexCount(); ++i)
    EXPECT_EQ(submeshCopy->Vertex(i), submesh->Vertex(i));
  for (unsigned int i = 0; i < submeshCopy->IndexCount(); ++i)
    EXPECT_EQ(submeshCopy->Index(i), submesh->Index(i));
  for (unsigned int i = 0; i < submeshCopy->NodeAssignmentsCount(); ++i)
  {
    common::NodeAssignment nodeCopy =
      submeshCopy->NodeAssignmentByIndex(i);
    EXPECT_EQ(nodeCopy.vertexIndex, 1u);
    EXPECT_EQ(nodeCopy.nodeIndex, 0u);
    EXPECT_DOUBLE_EQ(nodeCopy.weight, 0.5f);
  }
}

/////////////////////////////////////////////////
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
