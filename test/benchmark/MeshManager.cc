/*
 * Copyright (C) 2026 Open Source Robotics Foundation
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

#include <benchmark/benchmark.h>
#include <gtest/gtest.h>

#include "gz/common/testing/TestPaths.hh"
#include "gz/common/ColladaLoader.hh"
#include "gz/common/Mesh.hh"
#include "gz/common/MeshManager.hh"
#include "gz/common/testing/AutoLogFixture.hh"

using namespace gz;

void BM_MeshManager(benchmark::State &_st, const std::string &_meshFile)
{
  auto *mgr = common::MeshManager::Instance();
  auto iterations = _st.range(0);
  
  for (auto _ : _st)
  {
    for (int64_t ii = 0; ii < iterations; ++ii)
    {
      const common::Mesh *mesh = mgr->Load(common::testing::TestFile("data", _meshFile));

      // Remove meshes from MeshManager, else MeshManager "keeps" a copy of the mesh 
      // loaded the first time. Load time for subsequent iterations will be 
      // very fast because it'll be retrieving the mesh from an unordered_map and not 
      // actually using the loader to load the mesh
      _st.PauseTiming();
      mgr->RemoveAll();
      _st.ResumeTiming();
    }
  }
}

// NOLINTNEXTLINE
// 565 vertices, 1126 faces
BENCHMARK_CAPTURE(BM_MeshManager, cordless_drill_dae, "cordless_drill/meshes/cordless_drill.dae")
    ->Arg(1)
    ->Unit(benchmark::kMillisecond);

// 144 vertices, 284 faces
BENCHMARK_CAPTURE(BM_MeshManager, cylinder_animated_from_3ds_max_dae, "cylinder_animated_from_3ds_max.dae")
    ->Arg(1)
    ->Unit(benchmark::kMillisecond);

// 8 vertices, 12 faces
BENCHMARK_CAPTURE(BM_MeshManager, box_fbx, "box.fbx")
    ->Arg(1)
    ->Unit(benchmark::kMillisecond);

// collection of meshes, total 1718 vertices, 846 faces
BENCHMARK_CAPTURE(BM_MeshManager, fully_featured_glb, "fully_featured.glb")
    ->Arg(1)
    ->Unit(benchmark::kMillisecond);

// 8 vertices, 12 faces
BENCHMARK_CAPTURE(BM_MeshManager, box_obj, "box.obj")
    ->Arg(1)
    ->Unit(benchmark::kMillisecond);

// 8 vertices, 12 faces
BENCHMARK_CAPTURE(BM_MeshManager, cube_stl, "cube.stl")
    ->Arg(1)
    ->Unit(benchmark::kMillisecond);

// 24 vertices, 12 faces
BENCHMARK_CAPTURE(BM_MeshManager, PurpleCube_gltf, "gltf/PurpleCube.gltf")
    ->Arg(1)
    ->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();
