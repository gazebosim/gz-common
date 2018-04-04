
# Turn on C++11 support for the library
set(CMAKE_CXX_STANDARD 11)

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
    VideoEncoder.cc)

  # FIXME: Some tests cannot build on Windows
  list(REMOVE_ITEM gtest_sources
    GTSMeshUtils_TEST.cc
    Image_TEST.cc
    ImageHeightmap_TEST.cc
    VideoEncoder_TEST.cc)

endif()

if(NOT USE_EXTERNAL_TINYXML2)
  # If we are using our internal copy of tinyxml2, then add its
  # source file.
  message(STATUS "Adding tinyxml2 source files")
  list(APPEND sources ${CMAKE_SOURCE_DIR}/graphics/src/tinyxml2/tinyxml2.cpp)
endif()

# Create the library target
ign_add_library(${PROJECT_LIBRARY_TARGET_NAME} ${sources} ${extra_sources})

# Link the libraries that we always need
target_link_libraries(${PROJECT_LIBRARY_TARGET_NAME}
  PUBLIC
    ignition-math${IGN_MATH_VER}::ignition-math${IGN_MATH_VER}
  PRIVATE
    ${DL_TARGET})

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
    PRIVATE ${CMAKE_SOURCE_DIR}/graphics/src/tinyxml2)
endif()

# define of tinxml2 major version >= 6
# https://bitbucket.org/ignitionrobotics/ign-common/issues/28
if (NOT TINYXML2_VERSION VERSION_LESS "6.0.0")
  message(STATUS "TINYXML2_VERSION ${TINYXML2_VERSION} >= 6.0.0")
  target_compile_definitions(${PROJECT_LIBRARY_TARGET_NAME}
    PRIVATE "TINYXML2_MAJOR_VERSION_GE_6")
endif()

# Include the interface directories that we always need.
# Note that FreeImage, DL, and TINYXML2 are not included here, because their
# headers are only included in the source code, and do not need to be visible
# to projects that depend on ignition-common.
ign_target_interface_include_directories(${PROJECT_LIBRARY_TARGET_NAME}
  ignition-math${IGN_MATH_VER}::ignition-math${IGN_MATH_VER})

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
      FreeImage::FreeImage
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

if(TARGET UNIT_ColladaExporter_TEST)
  if(USE_EXTERNAL_TINYXML2)
    # The collada exporter test uses tinyxml2, so we must link it if we're using
    # and external copy.
    target_link_libraries(UNIT_ColladaExporter_TEST TINYXML2::TINYXML2)
  else()
    # If we are using the internal copy of tinyxml2, then the collada exporter
    # test needs to be pointed to the internal tinyxml2 include directory.
    target_include_directories(UNIT_ColladaExporter_TEST PRIVATE ${CMAKE_SOURCE_DIR}/graphics/src/tinyxml2)
    # We also need to add this as a source file to the collada exporter test's
    # target, because we do not export the tinyxml2 symbols that get generated
    # in the ignition-common library. Therefore, the collada exporter test
    # cannot link to those symbols and must generate its own.
    target_sources(UNIT_ColladaExporter_TEST PRIVATE ${CMAKE_SOURCE_DIR}/graphics/src/tinyxml2/tinyxml2.cpp)
  endif()
endif()

if(TARGET UNIT_PluginLoader_TEST)
  target_compile_definitions(UNIT_PluginLoader_TEST PRIVATE
    "IGN_COMMON_LIB_PATH=\"$<TARGET_FILE_DIR:${PROJECT_LIBRARY_TARGET_NAME}>\"")
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
