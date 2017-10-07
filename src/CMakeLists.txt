include (${project_cmake_dir}/Utils.cmake)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${tinyxml2_INCLUDE_DIRS})

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
  Plugin.cc
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
  PluginUtils_TEST.cc
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
  ${tinyxml2_LIBRARIES}
  ${uuid_LIBRARIES}
  )

if(NOT WIN32)
  target_link_libraries(${PROJECT_LIBRARY_TARGET_NAME}
    ${freeimage_LIBRARIES}
    ${gts_LIBRARIES}
    ${libavcodec_LIBRARIES}
    ${libavformat_LIBRARIES}
    ${libavutil_LIBRARIES}
    ${libavdevice_LIBRARIES}
    ${libswscale_LIBRARIES}
  )
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
  $<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR_FULL}>)

ign_install_library(${PROJECT_LIBRARY_TARGET_NAME} ${PROJECT_EXPORT_NAME})

#ign_add_library(ignition-common ${sources})
#ign_install_library(ignition-common)

#target_link_libraries(ignition-common
#  ${IGNITION-MATH_LIBRARIES}
#)

ign_build_tests(${gtest_sources})
