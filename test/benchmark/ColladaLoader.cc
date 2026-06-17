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
#include "gz/common/testing/AutoLogFixture.hh"

using namespace gz;

void BM_ColladaLoader(benchmark::State &_st, const std::string &_meshFile)
{
  common::ColladaLoader loader;

  auto iterations = _st.range(0);
  
  for (auto _ : _st)
  {
    for (int64_t ii = 0; ii < iterations; ++ii)
  {
    benchmark::DoNotOptimize(loader.Load(common::testing::TestFile("data", _meshFile)));
    }
  }
}

// NOLINTNEXTLINE
BENCHMARK_CAPTURE(BM_ColladaLoader, sydney_regatta, "sydney_regatta/meshes/sydney_regatta.dae")
    ->Arg(1)
    ->Unit(benchmark::kMillisecond);

BENCHMARK_CAPTURE(BM_ColladaLoader, lruav_base, "base.dae")
    ->Arg(1)
    ->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();
