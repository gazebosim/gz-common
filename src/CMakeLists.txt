include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  # Used to make test headers visible to the unit tests
  ${PROJECT_SOURCE_DIR}
  # Used to make test_config.h visible to the unit tests
  ${PROJECT_BINARY_DIR}
  ${TINYXML2_INCLUDE_DIRS})

set (sources
  Animation.cc
  Base64.cc
  Battery.cc
  BVHLoader.cc
  ColladaExporter.cc
  ColladaLoader.cc
  Console.cc
  Dem.cc
  Event.cc
  Filesystem.cc
  FilesystemBoost.cc
  KeyEvent.cc
  KeyFrame.cc
  Material.cc
  MaterialDensity.cc
  Mesh.cc
  MeshExporter.cc
  MeshManager.cc
  MouseEvent.cc
  NodeAnimation.cc
  NodeTransform.cc
  OBJLoader.cc
  PluginLoader.cc
  SemanticVersion.cc
  Skeleton.cc
  SkeletonAnimation.cc
  SkeletonNode.cc
  STLLoader.cc
  StringUtils.cc
  SystemPaths.cc
  SubMesh.cc
  SVGLoader.cc
  Time.cc
  Timer.cc
  URI.cc
  Util.cc
  Uuid.cc
  WorkerPool.cc
)

# FIXME: Some classes fail to build on Windows
if(NOT WIN32)
  set (sources ${sources}
    AudioDecoder.cc
    ffmpeg_inc.cc
    GTSMeshUtils.cc
    Image.cc
    ImageHeightmap.cc
    MeshCSG.cc
    Video.cc
    VideoEncoder.cc
  )
endif()

if (NOT USE_EXTERNAL_TINYXML2)
    list (APPEND sources ${CMAKE_SOURCE_DIR}/src/tinyxml2/tinyxml2.cpp)
endif()

set (gtest_sources
  Animation_TEST.cc
  Base64_TEST.cc
  Battery_TEST.cc
  ColladaExporter_TEST.cc
  ColladaLoader_TEST.cc
  Console_TEST.cc
  Dem_TEST.cc
  EnumIface_TEST.cc
  Event_TEST.cc
  Filesystem_TEST.cc
  KeyEvent_TEST.cc
  Material_TEST.cc
  MaterialDensity_TEST.cc
  Mesh_TEST.cc
  MeshManager_TEST.cc
  MouseEvent_TEST.cc
  MovingWindowFilter_TEST.cc
  OBJLoader_TEST.cc
  PluginLoader_TEST.cc
  SemanticVersion_TEST.cc
  StringUtils_TEST.cc
  SubMesh_TEST.cc
  SVGLoader_TEST.cc
  SystemPaths_TEST.cc
  Time_TEST.cc
  URI_TEST.cc
  Util_TEST.cc
  Uuid_TEST.cc
  WorkerPool_TEST.cc
)

# FIXME: Some classes fail to build on Windows
if(NOT WIN32)
  set (gtest_sources ${gtest_sources}
    #AudioDecoder_TEST.cc
    GTSMeshUtils_TEST.cc
    Image_TEST.cc
    ImageHeightmap_TEST.cc
    VideoEncoder_TEST.cc
  )
endif()

ign_add_library(${PROJECT_LIBRARY_TARGET_NAME} ${sources})
target_link_libraries(${PROJECT_LIBRARY_TARGET_NAME}
  ${IGNITION-MATH_LIBRARIES}
  ${TINYXML2_LIBRARIES}
  ${UUID_LIBRARIES}
  ${DL_LIBRARIES})

if(NOT MSVC)

  target_include_directories(${PROJECT_LIBRARY_TARGET_NAME}
    PRIVATE
    ${GTS_INCLUDE_DIRS}
    ${UUID_INCLUDE_DIRS}
    ${SWSCALE_INCLUDE_DIRS}
    ${AVDEVICE_INCLUDE_DIRS}
  )

  target_link_libraries(${PROJECT_LIBRARY_TARGET_NAME}
    ${FreeImage_LIBRARIES}
    ${GTS_LIBRARIES}
    ${AVCODEC_LIBRARIES}
    ${AVFORMAT_LIBRARIES}
    ${AVUTIL_LIBRARIES}
    ${AVDEVICE_LIBRARIES}
    ${SWSCALE_LIBRARIES}
  )

  target_compile_options(${PROJECT_LIBRARY_TARGET_NAME}
    PRIVATE
    ${GTS_CFLAGS})

endif()

if (UNIX AND NOT APPLE)
  target_link_libraries(${PROJECT_LIBRARY_TARGET_NAME} rt)

  # Need to add default visibility to ignition-msgs
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

# When the minimum CMake required version will be >= 3.1
# we could use the target_compile_features() command
# to enable C++11 support in a platform-independent way.
if(NOT MSVC)
  target_compile_options(${PROJECT_LIBRARY_TARGET_NAME} PUBLIC "-std=c++11")
endif()

target_include_directories(${PROJECT_LIBRARY_TARGET_NAME}
  PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${IGN_INCLUDE_INSTALL_DIR_FULL}>)

target_include_directories(${PROJECT_LIBRARY_TARGET_NAME} PRIVATE ${DL_INCLUDE_DIRS})

ign_install_library(${PROJECT_LIBRARY_TARGET_NAME} ${PROJECT_EXPORT_NAME})

#ign_add_library(ignition-common ${sources})
#ign_install_library(ignition-common)

#target_link_libraries(ignition-common
#  ${IGNITION-MATH_LIBRARIES}
#)

ign_build_tests(${gtest_sources})

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
