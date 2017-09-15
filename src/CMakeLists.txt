include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  # Used to make test headers visible to the unit tests
  ${PROJECT_SOURCE_DIR}
  # Used to make test_config.h visible to the unit tests
  ${PROJECT_BINARY_DIR}
  ${TINYXML2_INCLUDE_DIRS})

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

ign_add_library(${PROJECT_LIBRARY_TARGET_NAME} ${sources})

# Link the libraries that we always need
target_link_libraries(${PROJECT_LIBRARY_TARGET_NAME}
  ${IGNITION-MATH_LIBRARIES}
  ${FreeImage_LIBRARIES}
  ${TINYXML2_LIBRARIES}
  ${DL_LIBRARIES})

# Include the directories that we always need
target_include_directories(${PROJECT_LIBRARY_TARGET_NAME}
  PRIVATE
  ${IGNITION-MATH_INCLUDE_DIRS}
  ${FreeImage_INCLUDE_DIRS}
  ${TINYXML2_INCLUDE_DIRS}
  ${DL_INCLUDE_DIRS})

# This generator expression is necessary for multi-configuration generators,
# such as MSVC on Windows.
target_include_directories(${PROJECT_LIBRARY_TARGET_NAME}
  PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${IGN_INCLUDE_INSTALL_DIR_FULL}>)

# Handle non-Windows configuration settings
if(NOT WIN32)

  # Link the libraries that we don't expect to find on Windows
  target_link_libraries(${PROJECT_LIBRARY_TARGET_NAME}
    ${UUID_LIBRARIES}
    ${GTS_LIBRARIES}
    ${SWSCALE_LIBRARIES}
    ${AVDEVICE_LIBRARIES}
    ${AVFORMAT_LIBRARIES}
    ${AVCODEC_LIBRARIES}
    ${AVUTIL_LIBRARIES}
  )

  # Include the directories that we don't expect to find on Windows
  target_include_directories(${PROJECT_LIBRARY_TARGET_NAME}
    PRIVATE
    ${UUID_INCLUDE_DIRS}
    ${GTS_INCLUDE_DIRS}
    ${SWSCALE_INCLUDE_DIRS}
    ${AVDEVICE_INCLUDE_DIRS}
    ${AVFORMAT_INCLUDE_DIRS}
    ${AVCODEC_INCLUDE_DIRS}
    ${AVUTIL_INCLUDE_DIRS}
  )

  # Add compile options for the library
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


ign_install_library(${PROJECT_LIBRARY_TARGET_NAME} ${PROJECT_EXPORT_NAME})


ign_build_tests(
  TYPE UNIT
  SOURCES ${gtest_sources})

if(WIN32)
  if (IGN_BUILD_SYMLINK_TESTS_ON_WINDOWS)
    message(STATUS "")
    message(STATUS "You have opted to enable symlink tests on a Windows platform.")
    message(STATUS "The test UNIT_Filesystem_TEST will require elevated privileges")
    message(STATUS "in order to succeed. For more information, see the issue")
    message(STATUS "https://bitbucket.org/ignitionrobotics/ign-common/issues/21")
    message(STATUS "")
    target_compile_definitions(UNIT_Filesystem_TEST PRIVATE IGN_BUILD_SYMLINK_TESTS_ON_WINDOWS)
  endif ()
endif ()
