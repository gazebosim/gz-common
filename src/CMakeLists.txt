
# Collect source files into the "sources" variable and unit test files into the
# "gtest_sources" variable
ign_get_libsources_and_unittests(sources gtest_sources)

# FIXME: This class does not currently work
list(REMOVE_ITEM sources AudioDecoder.cc)
list(REMOVE_ITEM gtest_sources AudioDecoder_TEST.cc)

if(WIN32)

  # FIXME: Some classes fail to build on Windows
  list(REMOVE_ITEM sources
    AudioDecoder.cc
    ffmpeg_inc.cc
    GTSMeshUtils.cc
    Image.cc
    ImageHeightmap.cc
    MeshCSG.cc
    Video.cc
    VideoEncoder.cc
  )

  # FIXME: Some tests cannot build on Windows
  list(REMOVE_ITEM gtest_sources
    GTSMeshUtils_TEST.cc
    Image_TEST.cc
    ImageHeightmap_TEST.cc
    VideoEncoder_TEST.cc
  )

endif()

if(NOT USE_EXTERNAL_TINYXML2)
  list(APPEND sources tinyxml2/tinyxml2.cpp)
endif()

# Create the library target
ign_add_library(${PROJECT_LIBRARY_TARGET_NAME} ${sources})

# Link the libraries that we always need
target_link_libraries(${PROJECT_LIBRARY_TARGET_NAME}
  PUBLIC
    ignition-math3::ignition-math3
  PRIVATE
    FreeImage::FreeImage
    DL::DL)

if(USE_EXTERNAL_TINYXML2)
  # If we are using an external copy of tinyxml2, add its imported target
  target_link_libraries(${PROJECT_LIBRARY_TARGET_NAME}
    PRIVATE
      TINYXML2::TINYXML2)
else()
  # Otherwise, add its source directory to our target's PRIVATE include
  # directories. We do not want this to be visible to consumers of
  # ignition-common.
  target_include_directories(${PROJECT_LIBRARY_TARGET_NAME}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tinyxml2)
endif()

# Include the interface directories that we always need.
# Note that FreeImage, DL, and TINYXML2 are not included here, because their
# headers are only included in the source code, and do not need to be visible
# to projects that depend on ignition-common.
ign_target_interface_include_directories(${PROJECT_LIBRARY_TARGET_NAME}
  ignition-math3::ignition-math3)

# This generator expression is necessary for multi-configuration generators,
# such as MSVC on Windows, and also to ensure that our target exports the
# headers correctly
target_include_directories(${PROJECT_LIBRARY_TARGET_NAME}
  # This is the public ignition/common headers directory
  PUBLIC $<INSTALL_INTERFACE:${IGN_INCLUDE_INSTALL_DIR_FULL}>
  # This is the build directory version of the headers
  PRIVATE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)

# Handle non-Windows configuration settings
if(NOT WIN32)

  # Link the libraries that we don't expect to find on Windows
  target_link_libraries(${PROJECT_LIBRARY_TARGET_NAME}
    PUBLIC
      UUID::UUID
      SWSCALE::SWSCALE
      AVDEVICE::AVDEVICE
      AVFORMAT::AVFORMAT
      AVCODEC::AVCODEC
      AVUTIL::AVUTIL
    PRIVATE
      GTS::GTS)

  # Include the interface directories that we don't expect to find on Windows.
  # Note that GTS is not included here, because its headers are only used
  # internally by the source code and do not need to be visible to projects that
  # depend on ignition-common.
  ign_target_interface_include_directories(${PROJECT_LIBRARY_TARGET_NAME}
    UUID::UUID
    SWSCALE::SWSCALE
    AVDEVICE::AVDEVICE
    AVFORMAT::AVFORMAT
    AVCODEC::AVCODEC
    AVUTIL::AVUTIL)

  # Add compile options for the library. This should not be needed by projects
  # that depend on ignition-common, so we mark it PRIVATE
  target_compile_options(${PROJECT_LIBRARY_TARGET_NAME}
    PRIVATE
      ${GTS_CFLAGS})

endif()

if(UNIX AND NOT APPLE)

  # Need to add default visibility
  get_target_property(current_property ${PROJECT_LIBRARY_TARGET_NAME}
    COMPILE_FLAGS)

  # property non-existent or empty
  if(NOT current_property)
    set_target_properties(${PROJECT_LIBRARY_TARGET_NAME}
      PROPERTIES GENERATED TRUE
      COMPILE_FLAGS "-fvisibility=default")
  else()
    set_target_properties(${PROJECT_LIBRARY_TARGET_NAME}
      PROPERTIES COMPILE_FLAGS
      "${current_property} -fvisibility=default")
  endif()

endif()

# Turn on C++11 support for the library
set_property(TARGET ${PROJECT_LIBRARY_TARGET_NAME} PROPERTY CXX_STANDARD 11)

# Create installation instructions for the library target. This must be called
# in the same scope that the target is created.
ign_install_library()

# Build the unit tests
ign_build_tests(
  TYPE UNIT
  SOURCES ${gtest_sources}
  INCLUDE_DIRS
    # Used to make internal source file headers visible to the unit tests
    ${CMAKE_CURRENT_SOURCE_DIR}
    # Used to make test-directory headers visible to the unit tests
    ${PROJECT_SOURCE_DIR}
    # Used to make test_config.h visible to the unit tests
    ${PROJECT_BINARY_DIR})

if(USE_EXTERNAL_TINYXML2)
  # The collada exporter test uses tinyxml2, so we must link it if we're using
  # and external copy.
  target_link_libraries(UNIT_ColladaExporter_TEST TINYXML2::TINYXML2)
endif()

# Produce warning on Windows if the user has decided to turn on the symlink
# tests. In order for those tests to work, they will need to run the tests in
# administrative mode, or use some other workaround.
if(WIN32)
  if(IGN_BUILD_SYMLINK_TESTS_ON_WINDOWS)
    message(STATUS "")
    message(STATUS "You have opted to enable symlink tests on a Windows platform.")
    message(STATUS "The test UNIT_Filesystem_TEST will require elevated privileges")
    message(STATUS "in order to succeed. For more information, see the issue")
    message(STATUS "https://bitbucket.org/ignitionrobotics/ign-common/issues/21")
    message(STATUS "")
    target_compile_definitions(UNIT_Filesystem_TEST PRIVATE IGN_BUILD_SYMLINK_TESTS_ON_WINDOWS)
  endif()
endif()
