/*
 * Copyright (C) 2016 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <gts.h>

#include <string>

#include "ignition/common/Console.hh"
#include "ignition/common/Mesh.hh"
#include "ignition/common/SubMesh.hh"
#include "ignition/common/MeshCSG.hh"

using namespace ignition;
using namespace common;

//////////////////////////////////////////////////
MeshCSG::MeshCSG()
{
}

//////////////////////////////////////////////////
MeshCSG::~MeshCSG()
{
}

//////////////////////////////////////////////////
void MeshCSG::MergeVertices(GPtrArray * _vertices, double _epsilon)
{
  GPtrArray *array;
  GNode *kdtree;
  GtsVertex **verticesData = reinterpret_cast<GtsVertex **>(_vertices->pdata);
  array = g_ptr_array_new();
  for (unsigned int i = 0; i < _vertices->len; ++i)
    g_ptr_array_add(array, verticesData[i]);
  kdtree = gts_kdtree_new(array, nullptr);
  g_ptr_array_free(array, true);

  // for each vertex, do a bbox kdtree search to find nearby vertices within
  // _epsilon, merge vertices if they are within the bbox
  for (unsigned int i = 0; i < _vertices->len; ++i)
  {
    GtsVertex *v = reinterpret_cast<GtsVertex *>(verticesData[i]);

    // make sure vertex v is active (because they could be marked inactive
    // due to previous merge operation)
    if (!GTS_OBJECT (v)->reserved)
    {
      GtsBBox *bbox;
      GSList *selected, *j;

      // build bounding box
      bbox = gts_bbox_new(gts_bbox_class(),
          v,
          GTS_POINT(v)->x - _epsilon,
          GTS_POINT(v)->y - _epsilon,
          GTS_POINT(v)->z - _epsilon,
          GTS_POINT(v)->x + _epsilon,
          GTS_POINT(v)->y + _epsilon,
          GTS_POINT(v)->z + _epsilon);

      // select vertices which are inside bbox using kdtree
      j = selected = gts_kdtree_range(kdtree, bbox, nullptr);
      while (j)
      {
        GtsVertex *sv = reinterpret_cast<GtsVertex *>(j->data);
        // mark sv as inactive (merged)
        if (sv != v && !GTS_OBJECT(sv)->reserved)
          GTS_OBJECT(sv)->reserved = v;
        j = j->next;
      }
      g_slist_free(selected);
      gts_object_destroy(GTS_OBJECT(bbox));
    }
  }

  gts_kdtree_destroy(kdtree);

  // destroy inactive vertices
  // we want to control vertex destruction
  gts_allow_floating_vertices = true;
  for (unsigned int i = 0; i < _vertices->len; ++i)
  {
    GtsVertex * v = reinterpret_cast<GtsVertex *>(verticesData[i]);
    // v is inactive
    if (GTS_OBJECT(v)->reserved)
    {
      verticesData[i] =
          reinterpret_cast<GtsVertex *>(GTS_OBJECT(v)->reserved);
      gts_object_destroy(GTS_OBJECT(v));
    }
  }
  gts_allow_floating_vertices = false;
}

//////////////////////////////////////////////////
static int FillVertex(GtsPoint *_p, gpointer *_data)
{
  // create a vertex from GTS_POINT and add it to the submesh
  SubMesh *subMesh = reinterpret_cast<SubMesh *>(_data[0]);
  GHashTable* vIndex = reinterpret_cast<GHashTable *>(_data[2]);
  subMesh->AddVertex(GTS_POINT(_p)->x, GTS_POINT(_p)->y, GTS_POINT(_p)->z);
  // fill the hash table which will later be used for adding indices to the
  // submesh in the FillFace function.
  g_hash_table_insert(vIndex, _p,
      GUINT_TO_POINTER((*(reinterpret_cast<guint *>(_data[1])))++));
  return 0;
}

//////////////////////////////////////////////////
static int FillFace(GtsTriangle *_t, gpointer *_data)
{
  SubMesh *subMesh = reinterpret_cast<SubMesh *>(_data[0]);
  GHashTable* vIndex = reinterpret_cast<GHashTable *>(_data[2]);

  GtsVertex * v1, * v2, * v3;
  gts_triangle_vertices(_t, &v1, &v2, &v3);

  subMesh->AddIndex(GPOINTER_TO_UINT(g_hash_table_lookup(vIndex, v1)));
  subMesh->AddIndex(GPOINTER_TO_UINT(g_hash_table_lookup(vIndex, v2)));
  subMesh->AddIndex(GPOINTER_TO_UINT(g_hash_table_lookup(vIndex, v3)));
  return 0;
}

//////////////////////////////////////////////////////////////////////////
static int TriangleRevert(GtsTriangle *_t, void *)
{
  gts_triangle_revert(_t);
  return 0;
}

//////////////////////////////////////////////////
Mesh *MeshCSG::CreateBoolean(const Mesh *_m1, const Mesh *_m2, int _operation,
    const math::Pose3d &_offset)
{
  GtsSurface *s1, *s2, *s3;
  GtsSurfaceInter *si;
  GNode *tree1, *tree2;

  gboolean closed = true;
  bool isOpen1 = false;
  bool isOpen2 = false;

  s1 = gts_surface_new(gts_surface_class(), gts_face_class(), gts_edge_class(),
      gts_vertex_class());
  s2 = gts_surface_new(gts_surface_class(), gts_face_class(), gts_edge_class(),
      gts_vertex_class());
  s3 = gts_surface_new(gts_surface_class(), gts_face_class(), gts_edge_class(),
      gts_vertex_class());

  this->ConvertMeshToGTS(_m1, s1);

  if (_offset != math::Pose3d::Zero)
  {
    Mesh *m2 = new Mesh();
    for (unsigned int i = 0; i < _m2->SubMeshCount(); ++i)
    {
      SubMesh m2SubMesh;

      auto subMesh = _m2->SubMeshByIndex(i).lock();
      if (subMesh->VertexCount() <= 2)
        continue;
      for (unsigned int j = 0; j < subMesh->VertexCount(); ++j)
      {
        m2SubMesh.AddVertex(_offset.Pos() +
            _offset.Rot()*subMesh->Vertex(j));
      }
      for (unsigned int j = 0; j < subMesh->IndexCount(); ++j)
      {
        m2SubMesh.AddIndex(subMesh->Index(j));
      }

      m2->AddSubMesh(m2SubMesh);
    }
    this->ConvertMeshToGTS(m2, s2);
    delete m2;
  }
  else
  {
    this->ConvertMeshToGTS(_m2, s2);
  }

  // build bounding box tree for first surface
  tree1 = gts_bb_tree_surface(s1);
  isOpen1 = gts_surface_volume(s1) < 0. ? true : false;

  // build bounding box tree for second surface
  tree2 = gts_bb_tree_surface(s2);
  isOpen2 = gts_surface_volume(s2) < 0. ? true : false;

  si = gts_surface_inter_new(gts_surface_inter_class(), s1, s2, tree1, tree2,
      isOpen1, isOpen2);
  if (!gts_surface_inter_check(si, &closed))
  {
    ignerr << "si is not an orientable manifold\n";
    return nullptr;
  }

  if (!closed)
  {
    ignerr << "the intersection of " << _m1->Name() << " and "
        << _m2->Name() << " is not a closed curve\n";
    return nullptr;
  }

//  FILE *output1 = fopen("output3.gts", "w");
//  gts_surface_write(s1, output1);
//  fclose(output1);

//  FILE *output2 = fopen("output4.gts", "w");
//  gts_surface_write(s2, output2);
//  fclose(output2);

  if (_operation == MeshCSG::UNION)
  {
    gts_surface_inter_boolean(si, s3, GTS_1_OUT_2);
    gts_surface_inter_boolean(si, s3, GTS_2_OUT_1);
  }
  else if (_operation == MeshCSG::INTERSECTION)
  {
    gts_surface_inter_boolean(si, s3, GTS_1_IN_2);
    gts_surface_inter_boolean(si, s3, GTS_2_IN_1);
  }
  else if (_operation == MeshCSG::DIFFERENCE)
  {
    gts_surface_inter_boolean(si, s3, GTS_1_OUT_2);
    gts_surface_inter_boolean(si, s3, GTS_2_IN_1);
    gts_surface_foreach_face(si->s2, (GtsFunc) TriangleRevert, nullptr);
    gts_surface_foreach_face(s2, (GtsFunc) TriangleRevert, nullptr);
  }

//  FILE *output = fopen("output.gts", "w");
//  gts_surface_write(s3, output);
//  fclose(output);

  // create the boolean mesh
  Mesh *mesh = new Mesh();
  SubMesh subMesh;

  // fill the submesh with data generated by GTS
  unsigned int n = 0;
  gpointer data[3];
  GHashTable *vIndex = g_hash_table_new(nullptr, nullptr);

  data[0] = &subMesh;
  data[1] = &n;
  data[2] = vIndex;
  gts_surface_foreach_vertex(s3, (GtsFunc)FillVertex, data);
  n = 0;
  gts_surface_foreach_face(s3, (GtsFunc)FillFace, data);
  g_hash_table_destroy(vIndex);

  mesh->RecalculateNormals();

  // destroy surfaces
  gts_object_destroy(GTS_OBJECT(s1));
  gts_object_destroy(GTS_OBJECT(s2));
  gts_object_destroy(GTS_OBJECT(s3));
  gts_object_destroy(GTS_OBJECT(si));

  // destroy bounding box trees (including bounding boxes)
  gts_bb_tree_destroy(tree1, true);
  gts_bb_tree_destroy(tree2, true);

  mesh->AddSubMesh(subMesh);
  return mesh;
}

//////////////////////////////////////////////////
void MeshCSG::ConvertMeshToGTS(const Mesh *_mesh, GtsSurface *_surface)
{
  if (!_surface)
  {
    ignerr << _mesh->Name() << ": Surface is null\n";
//    _surface = gts_surface_new(gts_surface_class(), gts_face_class(),
//        gts_edge_class(), gts_vertex_class());
    return;
  }

  GPtrArray *vertices = g_ptr_array_new();

  for (unsigned int i = 0; i < _mesh->SubMeshCount(); ++i)
  {
    auto subMesh = _mesh->SubMeshByIndex(i).lock();
    unsigned int indexCount = subMesh->IndexCount();
    if (subMesh->VertexCount() <= 2)
      continue;

    for (unsigned int j = 0; j < subMesh->VertexCount(); ++j)
    {
      math::Vector3d vertex = subMesh->Vertex(j);
      g_ptr_array_add(vertices, gts_vertex_new(gts_vertex_class(), vertex.X(),
          vertex.Y(), vertex.Z()));
    }

    // merge duplicate vertices, otherwise gts produces undesirable results
    this->MergeVertices(vertices, 0.01);

    GtsVertex **verticesData =
        reinterpret_cast<GtsVertex **>(vertices->pdata);
    for (unsigned int j = 0; j < indexCount/3; ++j)
    {
      GtsEdge *e1 = GTS_EDGE(gts_vertices_are_connected(
          verticesData[subMesh->Index(3*j)],
          verticesData[subMesh->Index(3*j+1)]));
      GtsEdge *e2 = GTS_EDGE(gts_vertices_are_connected(
          verticesData[subMesh->Index(3*j+1)],
          verticesData[subMesh->Index(3*j+2)]));
      GtsEdge *e3 = GTS_EDGE(gts_vertices_are_connected(
          verticesData[subMesh->Index(3*j+2)],
          verticesData[subMesh->Index(3*j)]));
      if (e1 == nullptr && verticesData[subMesh->Index(3*j)]
          != verticesData[subMesh->Index(3*j+1)])
      {
        e1 = gts_edge_new(_surface->edge_class,
            verticesData[subMesh->Index(3*j)],
            verticesData[subMesh->Index(3*j+1)]);
      }
      if (e2 == nullptr && verticesData[subMesh->Index(3*j+1)]
          != verticesData[subMesh->Index(3*j+2)])
      {
        e2 = gts_edge_new(_surface->edge_class,
            verticesData[subMesh->Index(3*j+1)],
            verticesData[subMesh->Index(3*j+2)]);
      }
      if (e3 == nullptr && verticesData[subMesh->Index(3*j+2)]
          != verticesData[subMesh->Index(3*j)])
      {
        e3 = gts_edge_new(_surface->edge_class,
            verticesData[subMesh->Index(3*j+2)],
            verticesData[subMesh->Index(3*j)]);
      }
      if (e1 != nullptr && e2 != nullptr && e3 != nullptr)
      {
        gts_surface_add_face(_surface, gts_face_new(_surface->face_class, e1,
            e2, e3));
      }
      else
      {
        ignwarn << _mesh->Name() << ": Ignoring degenerate facet!";
      }
    }
  }
}
