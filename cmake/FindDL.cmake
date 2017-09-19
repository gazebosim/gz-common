#===============================================================================
# Copyright (C) 2017 Open Source Robotics Foundation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
########################################
# Find libdl
include(IgnPkgConfig)
ign_pkg_check_modules(DL dl)

if(NOT DL_FOUND)

  # If we cannot find the header or the library, we will switch this to false
  set(DL_FOUND true)

  # Search for the header
  find_path(DL_INCLUDE_DIRS dlfcn.h)
  if(DL_INCLUDE_DIRS)
    message(STATUS "Looking for dlfcn.h - found")
  else(DL_INCLUDE_DIRS)
    message(STATUS "Looking for dlfcn.h - not found")
    set(DL_FOUND false)
  endif()

  # Search for the library
  find_library(DL_LIBRARIES dl)
  if(DL_LIBRARIES)
    message(STATUS "Looking for libdl - found")
  else(DL_LIBRARIES)
    message(STATUS "Looking for libdl - not found")
    set(DL_FOUND false)
  endif()

  if(DL_FOUND)
    include(IgnImportTarget)
    ign_import_target(DL)
  endif()

endif()
