include (${project_cmake_dir}/Utils.cmake)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/boost)

set (headers
  Animation.hh
  AudioDecoder.hh
  BVHLoader.hh
  ColladaLoader.hh
  Color.hh
  Console.hh
  Exception.hh
  ffmpeg_inc.h
  #Image.hh
  KeyFrame.hh
  Material.hh
  Mesh.hh
  NodeTransform.hh
  SingletonT.hh
  Skeleton.hh
  SkeletonAnimation.hh
  SkeletonNode.hh
  SubMesh.hh
  System.hh
  Time.hh
  Timer.hh
  Util.hh
)

set (ign_headers "" CACHE INTERNAL "Ignition headers" FORCE)
foreach (hdr ${headers})
  APPEND_TO_CACHED_STRING(ign_headers
    "Ignition headers" "#include <ignition/${IGN_PROJECT_NAME}/${hdr}>\n")
endforeach()

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/ign_auto_headers.hh.in
  ${CMAKE_CURRENT_BINARY_DIR}/${IGN_PROJECT_NAME}.hh)

ign_install_includes("" ${CMAKE_CURRENT_BINARY_DIR}/${IGN_PROJECT_NAME}.hh)
ign_install_includes(${IGN_PROJECT_NAME} ${headers})
