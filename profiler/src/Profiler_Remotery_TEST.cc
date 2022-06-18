/*
 * Copyright (C) 2018 Open Source Robotics Foundation
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

#include "gz/common/Profiler.hh" // NOLINT(*)
#include <gtest/gtest.h> // NOLINT(*)

#include <atomic> // NOLINT(*)
#include <thread> // NOLINT(*)
#include "gz/common/Util.hh" // NOLINT(*)

using namespace gz;
using namespace common;

/////////////////////////////////////////////////
TEST(Profiler, ProfilerDisabled)
{
  EXPECT_TRUE(GZ_PROFILER_ENABLE);
  EXPECT_TRUE(GZ_PROFILER_VALID);
  EXPECT_EQ(Profiler::Instance()->ImplementationName(),
            "gz_profiler_remotery");
}
