/*
 * Copyright (C) 2021 Open Source Robotics Foundation
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
#ifndef IGNITION_COMMON_IGN_HH_
#define IGNITION_COMMON_IGN_HH_

#include "ignition/common/Export.hh"

/// \brief External hook to read the library version.
/// \return C-string representing the version. Ex.: 0.1.2
extern "C" IGNITION_COMMON_VISIBLE char* ignitionCommonVersion();

/// \brief Set console verbosity of the other commands in this library.
/// \param[in] _verbosity 0 to 4
extern "C" IGNITION_COMMON_VISIBLE void cmdVerbosity(
  const char* _verbosity);

/// \brief Set console verbosity of the other commands in this library.
/// \param[in] _fileName Name of the searched file.
/// \param[in] _envName Name of the environment variable that holds additional
/// search paths. The content of the variable is treated as a colon/semicolon
/// separated list.
/// \param[in] _defaultPath Other directories to search (searched after those
/// extracted from the environment variable). A colon/semicolon separated list.
/// \return Path to the found existing file, or empty string if the file could
/// not be found. The path will use native separators.
extern "C" IGNITION_COMMON_VISIBLE char* findFileInPathEnv(
  const char* _fileName, const char* _envName, const char* _defaultPath = "");

#endif
