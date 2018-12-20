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

#ifndef IGNITION_COMMON_PROFILER_HH_
#define IGNITION_COMMON_PROFILER_HH_

#include <memory>
#include <string>

#include <ignition/common/Export.hh>
#include <ignition/common/SingletonT.hh>

namespace ignition
{
  namespace common
  {
    class ProfilerImpl;

    class IGNITION_COMMON_VISIBLE Profiler: public virtual SingletonT<Profiler>
    {
      protected: Profiler();

      protected: ~Profiler();

      public: void SetThreadName(const std::string& name);

      public: void LogText(const std::string& text);

      public: void BeginSample(const std::string& name);

      public: void EndSample();

      private: ProfilerImpl* impl;

      private: friend class SingletonT<Profiler>;
    };

    class ScopedProfile
    {
      public: ScopedProfile(const std::string& name)
      {
        Profiler::Instance()->BeginSample(name);
      }

      public: ~ScopedProfile()
      {
        Profiler::Instance()->EndSample();
      }
    };
  }
}


#define IGN_PROFILE_THREAD_NAME(name) Profiler::Instance()->SetThreadName(name);
#define IGN_PROFILE_LOG_TEXT(name)    Profiler::Instance()->LogText(name);
#define IGN_PROFILE_BEGIN(name)       Profiler::Instance()->BeginSample(name)
#define IGN_PROFILE_END()             Profiler::Instance()->EndSample()

#define IGN_PROFILE_LL(name, line)    ScopedProfile _profile##line(name);
#define IGN_PROFILE_L(name, line)     IGN_PROFILE_LL(name, line);
#define IGN_PROFILE(name)             IGN_PROFILE_L(name, __LINE__);

#endif  // IGNITION_COMMON_PROFILER_HH_
