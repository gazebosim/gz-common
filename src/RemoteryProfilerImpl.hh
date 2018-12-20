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

#ifndef IGNITION_COMMON_REMOTERYPROFILERIMPL_HH_
#define IGNITION_COMMON_REMOTERYPROFILERIMPL_HH_

#include "ProfilerImpl.hh"

#include <string>

#include "RemoteryConfig.h"
#include "Remotery.h"

namespace ignition
{
  namespace common
  {
    class RemoteryProfilerImpl: public ProfilerImpl
    {
      public: RemoteryProfilerImpl();
      public: ~RemoteryProfilerImpl() final;
      public: void SetThreadName(const std::string& name) final;
      public: void LogText(const std::string& text) final;
      public: void BeginSample(const std::string& name) final;
      public: void EndSample() final;

      private: rmtSettings* settings;
      private: Remotery* rmt;
    };
  }
}

#endif  // IGNITION_COMMON_PROFILERIMPL_HH_

