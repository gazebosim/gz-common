cmake_minimum_required(VERSION 3.5.1 FATAL_ERROR)

#============================================================================
# Find ignition-cmake
#============================================================================
set(IGNITION_CMAKE_DIR "${CMAKE_CURRENT_LIST_DIR}/cmake/")
list(APPEND CMAKE_MODULE_PATH "${IGNITION_CMAKE_DIR}")

include(IgnCMake)

#============================================================================
# Set up the project
#============================================================================
ign_configure_project(common 0 4 0)

#============================================================================
# Set project-specific options
#============================================================================

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
  # This is always false for Windows, so we can avoid trying to search for a
  # system installation of tinyxml2.
  set(USE_EXTERNAL_TINYXML2 false)
endif()


#============================================================================
# Search for project-specific dependencies
#============================================================================
message (STATUS "\n\n-- ====== Finding 3rd Party Packages ======")

#--------------------------------------
# Find ignition-math
ign_find_package(ignition-math3 REQUIRED)
include_directories(${IGNITION-MATH_INCLUDE_DIRS})

#--------------------------------------
# Find Freeimage
ign_find_package(FreeImage VERSION 3.9 REQUIRED)

#--------------------------------------
# Find Tinyxml2
if(USE_EXTERNAL_TINYXML2)
  ign_find_package(TINYXML2 REQUIRED PRETTY tinxyml2)
else()
  # Needed in WIN32 since in UNIX the flag is added in the code installed
  message (STATUS "Skipping search for tinyxml2")
  set(TINYXML2_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/src/tinyxml2")
  set(TINYXML2_LIBRARIES "")
  set(TINYXML2_LIBRARY_DIRS "")
endif()

#--------------------------------------
# Find libdl
ign_find_package(DL REQUIRED PURPOSE "Required for plugins")

#--------------------------------------
# Find dependencies that we ignore for Visual Studio
if(NOT MSVC)

  #------------------------------------
  # Find uuid
  ign_find_package(UUID REQUIRED PRETTY uuid)

  #------------------------------------
  # Find GNU Triangulation Surface Library
  ign_find_package(GTS REQUIRED PRETTY gts
                   PURPOSE "GNU Triangulation Surface library")

  #------------------------------------
  # Find libswscale
  ign_find_package(SWSCALE REQUIRED PRETTY libswscale)

  #------------------------------------
  # Find avdevice
  ign_find_package(AVDEVICE VERSION 56.4.100 REQUIRED PRETTY libavdevice)

  #------------------------------------
  # Find avformat
  ign_find_package(AVFORMAT REQUIRED PRETTY libavformat)

  #------------------------------------
  # Find avcodec
  find_package(AVCODEC REQUIRED PRETTY libavcodec)

endif()

message (STATUS "-------------------------------------------\n")


#============================================================================
# Configure the build
#============================================================================
ign_configure_build(QUIT_IF_BUILD_ERRORS)


#============================================================================
# Create package information
#============================================================================
ign_create_packages()
