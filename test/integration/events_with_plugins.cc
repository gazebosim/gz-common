/*
 * Copyright (C) 2022 Open Source Robotics Foundation
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
// dlfcn is safe to include since this test does not run on windows (see
// CMakeLists.txt)
#include <dlfcn.h>
#include <gz/common/Event.hh>
#include <gz/utils/ExtraTestMacros.hh>

#include "plugins/EventEmitterPlugin.hh"

using namespace gz;

TEST(EventWithPluginsTest, EventDestruction)
{
  // EventEmitterPlugin_LIB is defined in the CMake
  void *handle = dlopen(EventEmitterPlugin_LIB, RTLD_LAZY | RTLD_LOCAL);
  auto createEvent =
      reinterpret_cast<common::Event *(*)()>(dlsym(handle, "createEvent"));
  {
    // We use a unique_ptr instead of a raw delete call to avoid a compiler 
    // warning that `common::Event` does not have a virtual destructor.
    std::unique_ptr<common::Event> event;
    event.reset(createEvent());
    // This dynamic_cast is important to replicate how event's are signaled in
    // downstream applications
    auto *eventTyped = dynamic_cast<TestEvent *>(event.get());
    ASSERT_NE(nullptr, eventTyped);
    eventTyped->Signal();

    dlclose(handle);
  }
  SUCCEED();
}
