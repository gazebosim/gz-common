include (${project_cmake_dir}/Utils.cmake)
include (CheckCXXSourceCompiles)

include (${project_cmake_dir}/FindOS.cmake)
include (FindPkgConfig)
include (${project_cmake_dir}/FindFreeimage.cmake)

########################################
# Find ignition math in unix platforms
# In Windows we expect a call from configure.bat script with the paths
if (NOT WIN32)
  find_package(ignition-math2 QUIET)
  if (NOT ignition-math2_FOUND)
    message(STATUS "Looking for ignition-math2-config.cmake - not found")
    BUILD_ERROR ("Missing: Ignition math2 library.")
  else()
    message(STATUS "Looking for ignition-math2-config.cmake - found")
  endif()
endif()

########################################
# Include man pages stuff
include (${project_cmake_dir}/Ronn2Man.cmake)
add_manpage_target()

########################################
# Find Boost, if not specified manually
find_package(Boost ${MIN_BOOST_VERSION} REQUIRED filesystem system)
if (NOT Boost_FOUND)
  BUILD_ERROR ("Boost not found. Please install version "
    "${MIN_BOOST_VERSION} or higher.")
endif()
