cmake_minimum_required(VERSION 3.10.2 FATAL_ERROR)

#============================================================================
# Initialize the project
#============================================================================
project(gz-common5 VERSION 5.0.0)
set(GZ_COMMON_VER ${PROJECT_VERSION_MAJOR})

#============================================================================
# Find gz-cmake
#============================================================================
find_package(gz-cmake3 REQUIRED)
set(GZ_CMAKE_VER ${gz-cmake3_VERSION_MAJOR})

#============================================================================
# Configure the project
#============================================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

gz_configure_project(VERSION_SUFFIX pre1)

#============================================================================
# Set project-specific options
#============================================================================
#--------------------------------------
# Option: Should Windows test symlinking?
if(WIN32)
  option(
    GZ_BUILD_SYMLINK_TESTS_ON_WINDOWS
    "Creating symlinks requires special permissions on Windows, so those tests are disabled by default"
    false)
endif()

#--------------------------------------
# Option: Should we use our internal copy of tinyxml2?
if(UNIX OR APPLE)
  option(USE_EXTERNAL_TINYXML2 "Use a system-installed version of tinyxml2" ON)
elseif(WIN32)
  # For backward compatibility, this option on Windows by default is OFF
  option(USE_EXTERNAL_TINYXML2 "Use a system-installed version of tinyxml2" OFF)
endif()


#============================================================================
# Search for project-specific dependencies
#============================================================================
message(STATUS "\n\n-- ====== Finding Dependencies ======")

#--------------------------------------
# Find gz-math
gz_find_package(gz-math7 REQUIRED_BY geospatial graphics events)
set(GZ_MATH_VER ${gz-math7_VERSION_MAJOR})

#--------------------------------------
# Find gz-utils
gz_find_package(gz-utils2 REQUIRED)
set(GZ_UTILS_VER ${gz-utils2_VERSION_MAJOR})

#--------------------------------------
# Find Tinyxml2
if(USE_EXTERNAL_TINYXML2)
  gz_find_package(TINYXML2 PRETTY tinyxml2
    REQUIRED_BY graphics
    PRIVATE_FOR graphics)
else()
  message(STATUS "Skipping search for tinyxml2 and using an internal version\n")
endif()

#--------------------------------------
# Find libdl
gz_find_package(
  DL REQUIRED PRIVATE PRETTY libdl PURPOSE "Required for plugins")

#--------------------------------------
# Find dependencies that we ignore for Visual Studio
if(NOT MSVC)

  #------------------------------------
  # Find uuid
  gz_find_package(UUID REQUIRED PRETTY uuid)

endif()

#------------------------------------
# Find Freeimage
gz_find_package(FreeImage VERSION 3.9
  REQUIRED_BY graphics
  PRIVATE_FOR graphics)

#------------------------------------
# Find GNU Triangulation Surface Library
gz_find_package(
  GTS PRETTY gts PURPOSE "GNU Triangulation Surface library"
  REQUIRED_BY graphics
  PRIVATE_FOR graphics)

#------------------------------------
# Find GDAL
gz_find_package(GDAL VERSION 3.0
  PKGCONFIG gdal
  PRIVATE_FOR geospatial
  REQUIRED_BY geospatial)

#------------------------------------
# Find libswscale
gz_find_package(SWSCALE REQUIRED_BY av PRETTY libswscale)

#------------------------------------
# Find avdevice
gz_find_package(AVDEVICE VERSION 56.4.100 REQUIRED_BY av PRETTY libavdevice)

#------------------------------------
# Find avformat
gz_find_package(AVFORMAT REQUIRED_BY av PRETTY libavformat)

#------------------------------------
# Find avcodec
gz_find_package(AVCODEC REQUIRED_BY av PRETTY libavcodec)

#------------------------------------
# Find avutil
gz_find_package(AVUTIL REQUIRED_BY av PRETTY libavutil)

#------------------------------------
# Find assimp
gz_find_package(GzAssimp REQUIRED_BY graphics PRETTY assimp)

#------------------------------------
# Find csv
gz_find_package(CSV REQUIRED PRIVATE PRETTY libcsv PKGCONFIG_LIB csv)

message(STATUS "-------------------------------------------\n")


#============================================================================
# Configure the build
#============================================================================
configure_file("${PROJECT_SOURCE_DIR}/cppcheck.suppress.in"
               ${PROJECT_BINARY_DIR}/cppcheck.suppress)

gz_configure_build(QUIT_IF_BUILD_ERRORS
  COMPONENTS av events geospatial graphics profiler testing)

#============================================================================
# Create package information
#============================================================================
gz_create_packages()


#============================================================================
# Create documentation
#============================================================================
configure_file(${CMAKE_SOURCE_DIR}/api.md.in ${CMAKE_BINARY_DIR}/api.md)
configure_file(${CMAKE_SOURCE_DIR}/tutorials.md.in ${CMAKE_BINARY_DIR}/tutorials.md)

gz_create_docs(
  API_MAINPAGE_MD "${CMAKE_BINARY_DIR}/api.md"
  TUTORIALS_MAINPAGE_MD "${CMAKE_BINARY_DIR}/tutorials.md"
  TAGFILES
    "${GZ-MATH_DOXYGEN_TAGFILE} = ${GZ-MATH_API_URL}"
  )
