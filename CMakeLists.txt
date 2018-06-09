cmake_minimum_required(VERSION 3.5.1 FATAL_ERROR)

#============================================================================
# Initialize the project
#============================================================================
project(ignition-common2 VERSION 2.0.0)

#============================================================================
# Find ignition-cmake
#============================================================================
find_package(ignition-cmake1 REQUIRED)

#============================================================================
# Configure the project
#============================================================================
ign_configure_project(VERSION_SUFFIX pre1)

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
ign_find_package(ignition-math5 REQUIRED_BY graphics events)
set(IGN_MATH_VER ${ignition-math5_VERSION_MAJOR})

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
ign_configure_build(QUIT_IF_BUILD_ERRORS
  COMPONENTS av events graphics)


#============================================================================
# Create package information
#============================================================================
ign_create_packages()


#============================================================================
# Create documentation
#============================================================================
ign_create_docs(
  TAGFILES
    "${IGNITION-MATH_DOXYGEN_TAGFILE} = ${IGNITION-MATH_API_URL}"
  )
