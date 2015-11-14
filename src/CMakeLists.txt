include (${project_cmake_dir}/Utils.cmake)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# include_directories(${tinyxml_INCLUDE_DIRS})
# link_directories(${tinyxml_LIBRARY_DIRS})

if (NOT USE_EXTERNAL_TINYXML2)
  include_directories (${CMAKE_SOURCE_DIR}/deps/win/tinyxml2)
  set (sources ${sources}
       ${CMAKE_SOURCE_DIR}/deps/win/tinyxml2/tinyxml2.cpp)
endif()

set (sources
  Animation.cc
  AudioDecoder.cc
  BVHLoader.cc
  ColladaLoader.cc
  Color.cc
  Console.cc
  Exception.cc
  Image.cc
  KeyFrame.cc
  Mesh.cc
  Material.cc
  Mesh.cc
  NodeTransform.cc
  Skeleton.cc
  SkeletonAnimation.cc
  SkeletonNode.cc
  SubMesh.cc
  Time.cc
  Timer.cc
  Util.cc
)

set (gtest_sources
  Animation_TEST.cc
  #AudioDecoder_TEST.cc
  ColladaLoader_TEST.cc
  Color_TEST.cc
  Console_TEST.cc
  Exception_TEST.cc
  #Image_TEST.cc
  Material_TEST.cc
  Mesh_TEST.cc
  SubMesh_TEST.cc
  Time_TEST.cc
  Util_TEST.cc
)

ign_add_library(ignition-common ${sources})
ign_install_library(ignition-common)

target_link_libraries(ignition-common
  ${IGNITION-MATH_LIBRARIES}
  ${Boost_LIBRARIES}
  ${freeimage_LIBRARIES}
  #${tinyxml_LIBRARIES}
  ${tinyxml2_LIBRARIES}
)

ign_build_tests(${gtest_sources})
