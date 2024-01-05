cmake_minimum_required(VERSION 3.10.2 FATAL_ERROR)

#============================================================================
# Initialize the project
#============================================================================
project(ignition-common3 VERSION 3.17.0)

#============================================================================
# Find ignition-cmake
#============================================================================
find_package(ignition-cmake2 2.14.0 REQUIRED COMPONENTS utilities)
set(IGN_CMAKE_VER ${ignition-cmake2_VERSION_MAJOR})

#============================================================================
# Configure the project
#============================================================================
ign_configure_project(
  REPLACE_IGNITION_INCLUDE_PATH gz/common
)

#============================================================================
# Set project-specific options
#============================================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#--------------------------------------
# Option: Should Windows test symlinking?
if(WIN32)
  option(
    IGN_BUILD_SYMLINK_TESTS_ON_WINDOWS
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
# Find ignition-math
ign_find_package(ignition-math6 REQUIRED_BY graphics events)
set(IGN_MATH_VER ${ignition-math6_VERSION_MAJOR})

#--------------------------------------
# Find Tinyxml2
if(USE_EXTERNAL_TINYXML2)
  ign_find_package(TINYXML2 PRETTY tinyxml2
    REQUIRED_BY graphics
    PRIVATE_FOR graphics)
else()
  message(STATUS "Skipping search for tinyxml2 and using an internal version\n")
endif()

#--------------------------------------
# Find libdl
ign_find_package(
  DL REQUIRED PRIVATE PRETTY libdl PURPOSE "Required for plugins")

#--------------------------------------
# Find dependencies that we ignore for Visual Studio
if(NOT MSVC)

  #------------------------------------
  # Find uuid
  ign_find_package(UUID REQUIRED PRETTY uuid)

endif()

#------------------------------------
# Find Freeimage
ign_find_package(FreeImage VERSION 3.9
  REQUIRED_BY graphics
  PRIVATE_FOR graphics)

#------------------------------------
# Find GNU Triangulation Surface Library
ign_find_package(
  GTS PRETTY gts PURPOSE "GNU Triangulation Surface library"
  REQUIRED_BY graphics
  PRIVATE_FOR graphics)

#------------------------------------
# Find libswscale
ign_find_package(SWSCALE REQUIRED_BY av PRETTY libswscale)

#------------------------------------
# Find avdevice
ign_find_package(AVDEVICE VERSION 56.4.100 REQUIRED_BY av PRETTY libavdevice)

#------------------------------------
# Find avformat
ign_find_package(AVFORMAT REQUIRED_BY av PRETTY libavformat)

#------------------------------------
# Find avcodec
ign_find_package(AVCODEC REQUIRED_BY av PRETTY libavcodec)

#------------------------------------
# Find avutil
ign_find_package(AVUTIL REQUIRED_BY av PRETTY libavutil)


message(STATUS "-------------------------------------------\n")


#============================================================================
# Configure the build
#============================================================================
configure_file("${PROJECT_SOURCE_DIR}/cppcheck.suppress.in"
               ${PROJECT_BINARY_DIR}/cppcheck.suppress)

ign_configure_build(QUIT_IF_BUILD_ERRORS
  COMPONENTS av events graphics profiler)

#============================================================================
# Create package information
#============================================================================
ign_create_packages()


#============================================================================
# Create documentation
#============================================================================
configure_file(${CMAKE_SOURCE_DIR}/api.md.in ${CMAKE_BINARY_DIR}/api.md)
configure_file(${CMAKE_SOURCE_DIR}/tutorials.md.in ${CMAKE_BINARY_DIR}/tutorials.md)

ign_create_docs(
  API_MAINPAGE_MD "${CMAKE_BINARY_DIR}/api.md"
  TUTORIALS_MAINPAGE_MD "${CMAKE_BINARY_DIR}/tutorials.md"
  TAGFILES
    "${IGNITION-MATH_DOXYGEN_TAGFILE} = ${IGNITION-MATH_API_URL}"
  )
