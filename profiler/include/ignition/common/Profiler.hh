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

#include <gz/common/Profiler.hh>
#include <ignition/common/config.hh>

#define IGN_PROFILER_ENABLE GZ_PROFILER_ENABLE

#define IGN_PROFILE_THREAD_NAME(name)  GZ_PROFILE_THREAD_NAME(name)
#define IGN_PROFILE_LOG_TEXT(name)     GZ_PROFILE_LOG_TEXT(name)
#define IGN_PROFILE_BEGIN(name)        GZ_PROFILE_BEGIN(name)
#define IGN_PROFILE_END()              GZ_PROFILE_END()
#define IGN_PROFILE_L(name, line)      GZ_PROFILE_L(name, line)
#define IGN_PROFILE(name)              GZ_PROFILE(name)

#define IGN_PROFILER_VALID GZ_PROFILER_VALID
