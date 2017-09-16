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
# An alternative to pkg_check_modules that creates an "imported target" which
# helps us to make relocatable packages.
# TODO: When we require cmake-3.6+, we should remove this function and just use
#       the standard pkg_check_modules, which provides an option called
#       IMPORTED_TARGET that will create the imported targets the way we do here
#
macro(ign_pkg_check_modules package)

  find_package(PkgConfig)
  pkg_check_modules(${package} ${ARGN})

  if(${package}_FOUND AND NOT TARGET ${package})

    # For some reason, pkg_check_modules does not provide complete paths to the
    # libraries it returns, even though find_package is conventionally supposed
    # to provide complete library paths. Having only the library name is harmful
    # to the ign_create_imported_target macro, so we will change the variable to
    # give it complete paths.
    #
    # TODO: How would we deal with multiple modules that are in different
    # directories? How does cmake-3.6+ handle that situation?
    _ign_pkgconfig_find_libraries(
      ${package}_LIBRARIES
      ${package}
      "${${package}_LIBRARIES}"
      "${${package}_LIBRARY_DIRS}")

    ign_create_imported_target(${package})

  endif()

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(
    ${package}
    REQUIRED_VARS ${package}_FOUND)

endmacro()

# Based on discussion here: https://cmake.org/Bug/view.php?id=15804
# and a patch written by Sam Thursfield
function(_ign_pkgconfig_find_libraries output_var package library_names library_dirs)

  foreach(libname ${library_names})

    find_library(
      ${package}_LIBRARY_${libname}
      ${libname}
      PATHS ${library_dirs})

    list(APPEND library_paths "${${package}_LIBRARY_${libname}}")

  endforeach()

  set(${output_var} ${library_paths} PARENT_SCOPE)

endfunction()

macro(ign_create_imported_target package)

  # Link against this "imported" target by saying
  # target_link_libraries(mytarget package::package), instead of linking
  # against the variable package_LIBRARIES with the old-fashioned
  # target_link_libraries(mytarget ${package_LIBRARIES}
  add_library(${package}::${package} IMPORTED SHARED)

  message(STATUS "${package}_LIBRARIES:${${package}_LIBRARIES}")
  if(${package}_LIBRARIES)
    _ign_sort_libraries(${package} ${${package}_LIBRARIES})
  endif()

  message(STATUS "${package}_INCLUDE_DIRS:${${package}_INCLUDE_DIRS}")
  foreach(${package}_inc ${${package}_INCLUDE_DIRS})
    set_target_properties(${package}::${package} PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${${package}_inc}")
  endforeach()

  message(STATUS "${package}_CFLAGS:${${package}_CFLAGS}")
  if(${package}_CFLAGS)
    set_target_properties(${package}::${package} PROPERTIES
      INTERFACE_COMPILE_OPTIONS "${${package}_CFLAGS}")
  endif()

  # What about linker flags? Is there no target property for that?

endmacro()

# This is an awkward hack to give the package both an IMPORTED_LOCATION and
# a set of INTERFACE_LIBRARIES in the event that PkgConfig returns multiple
# libraries for this package. It seems that IMPORTED_LOCATION cannot support
# specifying multiple libraries, so if we have multiple libraries, we need to
# pass them into LINK_INTERFACE_LIBRARIES. However, if IMPORTED_LOCATION is
# missing from the target, the dependencies do not get configured correctly by
# the generator expressions, and the build system will try to link to a nonsense
# garbage file.
function(_ign_sort_libraries package first_lib)

  set_target_properties(${package}::${package} PROPERTIES
    IMPORTED_LOCATION "${first_lib}")

  foreach(extra_lib ${ARGN})
    set_target_properties(${package}::${package} PROPERTIES
      LINK_INTERFACE_LIBRARIES "${extra_lib}")
  endforeach()

endfunction()
