/*
 * Copyright (C) 2017 Open Source Robotics Foundation
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

#define SUPPRESS_IGNITION_HEADER_DEPRECATION

#include "ignition/common/PluginMacros.hh"
#include "gz/utils/SuppressWarning.hh"

GZ_UTILS_WARN_IGNORE__DEPRECATED_DECLARATION

extern "C" {
  std::size_t DETAIL_IGN_PLUGIN_VISIBLE IGNCOMMONPluginInfoSize =
    sizeof(gz::common::PluginInfo);

  std::size_t DETAIL_IGN_PLUGIN_VISIBLE IGNCOMMONPluginInfoAlignment =
    alignof(gz::common::PluginInfo);

  int DETAIL_IGN_PLUGIN_VISIBLE IGNCOMMONPluginAPIVersion = -1;
}

extern "C" std::size_t DETAIL_IGN_PLUGIN_VISIBLE IGNCOMMONMultiPluginInfo(
    void *, const std::size_t, const std::size_t)
{
  return 0u;
}

GZ_UTILS_WARN_RESUME__DEPRECATED_DECLARATION

#undef SUPPRESS_IGNITION_HEADER_DEPRECATION
