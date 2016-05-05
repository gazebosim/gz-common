include (${project_cmake_dir}/Utils.cmake)
include (CheckCXXSourceCompiles)

include (${project_cmake_dir}/FindOS.cmake)
include (FindPkgConfig)
include (${project_cmake_dir}/FindFreeimage.cmake)

########################################
# Find ignition math
find_package(ignition-math2 QUIET)
if (NOT ignition-math2_FOUND)
  message(STATUS "Looking for ignition-math2-config.cmake - not found")
  BUILD_ERROR ("Missing: Ignition math2 library.")
else()
  message(STATUS "Looking for ignition-math2-config.cmake - found")
endif()

########################################
# Include man pages stuff
include (${project_cmake_dir}/Ronn2Man.cmake)
add_manpage_target()

########################################
# Find Boost, if not specified manually
include(FindBoost)
find_package(Boost ${MIN_BOOST_VERSION} REQUIRED filesystem system)
if (NOT Boost_FOUND)
  BUILD_ERROR ("Boost not found. Please install version "
    "${MIN_BOOST_VERSION} or higher.")
endif()

#################################################
# Find tinyxml2. Only debian distributions package tinyxml with a pkg-config
# Use pkg_check_modules and fallback to manual detection
# (needed, at least, for MacOS)

set (tinyxml2_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/deps/tinyxml2")
set (tinyxml2_LIBRARIES "")
set (tinyxml2_LIBRARY_DIRS "")

# Macro to check for visibility capability in compiler
# Original idea from: https://gitorious.org/ferric-cmake-stuff/ 
macro (check_gcc_visibility)
  include (CheckCXXCompilerFlag)
  check_cxx_compiler_flag(-fvisibility=hidden GCC_SUPPORTS_VISIBILITY)
endmacro()
