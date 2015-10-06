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

if (USE_EXTERNAL_TINYXML)
  #################################################
  # Find tinyxml. Only debian distributions package tinyxml with a pkg-config
  # Use pkg_check_modules and fallback to manual detection (needed, at least, for MacOS)
  pkg_check_modules(tinyxml tinyxml)
  if (NOT tinyxml_FOUND)
    find_path (tinyxml_include_dirs tinyxml.h ${tinyxml_include_dirs} ENV CPATH)
    find_library(tinyxml_LIBRARIES NAMES tinyxml)
    set (tinyxml_FAIL False)
    if (NOT tinyxml_include_dirs)
      message (STATUS "Looking for tinyxml headers - not found")
      set (tinyxml_FAIL True)
    endif()
    if (NOT tinyxml_LIBRARIES)
      message (STATUS "Looking for tinyxml library - not found")
      set (tinyxml_FAIL True)
    endif()
  endif()

  if (tinyxml_FAIL)
    message (STATUS "Looking for tinyxml.h - not found")
    BUILD_ERROR("Missing: tinyxml")
  endif()
else()
  # Needed in WIN32 since in UNIX the flag is added in the code installed
  add_definitions(-DTIXML_USE_STL)
  include_directories (${PROJECT_SOURCE_DIR}/src/win/tinyxml)
  set (tinyxml_LIBRARIES "tinyxml")
  set (tinyxml_LIBRARY_DIRS "")
endif()
