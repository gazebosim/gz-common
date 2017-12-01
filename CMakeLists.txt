cmake_minimum_required(VERSION 3.5.1 FATAL_ERROR)

#============================================================================
# Find ignition-cmake
#============================================================================
find_package(ignition-cmake0 REQUIRED)

#============================================================================
# Set up the project
#============================================================================
ign_configure_project(common 0.5.0 VERSION_SUFFIX pre1)


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
message(STATUS "\n\n-- ====== Finding Dependencies ======")

#--------------------------------------
# Find ignition-math
set(IGN_MATH_VER 4)
ign_find_package(ignition-math${IGN_MATH_VER} REQUIRED)

#--------------------------------------
# Find Tinyxml2
if(USE_EXTERNAL_TINYXML2)
  ign_find_package(TINYXML2 REQUIRED PRIVATE PRETTY tinyxml2)
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

  #--------------------------------------
  # Find Freeimage
  ign_find_package(FreeImage VERSION 3.9 REQUIRED PRIVATE PKGCONFIG_IGNORE)

  #------------------------------------
  # Find uuid
  ign_find_package(UUID REQUIRED PRETTY uuid)

  #------------------------------------
  # Find GNU Triangulation Surface Library
  ign_find_package(
    GTS REQUIRED PRETTY gts PURPOSE "GNU Triangulation Surface library")

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
  ign_find_package(AVCODEC REQUIRED PRETTY libavcodec)

  #------------------------------------
  # Find avutil
  ign_find_package(AVUTIL REQUIRED PRETTY libavutil)

endif()

message(STATUS "-------------------------------------------\n")


#============================================================================
# Configure the build
#============================================================================
ign_configure_build(QUIT_IF_BUILD_ERRORS)


#============================================================================
# Create package information
#============================================================================
ign_create_packages()
