include (${project_cmake_dir}/Utils.cmake)

set (headers
  Animation.hh
  Base64.hh
  Battery.hh
  BVHLoader.hh
  ColladaExporter.hh
  ColladaLoader.hh
  Console.hh
  Dem.hh
  EnumIface.hh
  Event.hh
  Filesystem.hh
  HeightmapData.hh
  KeyEvent.hh
  KeyFrame.hh
  Material.hh
  MaterialDensity.hh
  Mesh.hh
  MeshExporter.hh
  MeshLoader.hh
  MeshManager.hh
  MouseEvent.hh
  MovingWindowFilter.hh
  NodeAnimation.hh
  NodeTransform.hh
  OBJLoader.hh
  Plugin.hh
  PluginInfo.hh
  PluginLoader.hh
  PluginMacros.hh
  SemanticVersion.hh
  SingletonT.hh
  Skeleton.hh
  SkeletonAnimation.hh
  SkeletonNode.hh
  StringUtils.hh
  SubMesh.hh
  System.hh
  SystemPaths.hh
  STLLoader.hh
  SVGLoader.hh
  Time.hh
  Timer.hh
  Types.hh
  Util.hh
  URI.hh
  Uuid.hh
  WorkerPool.hh

  detail/Plugin.hh
  detail/PluginMacros.hh
  detail/SuppressWarning.hh
)

# FIXME: Some classes fail to build on Windows
if(NOT WIN32)
  set (headers ${headers}
    AudioDecoder.hh
    ffmpeg_inc.hh
    GTSMeshUtils.hh
    Image.hh
    ImageHeightmap.hh
    MeshCSG.hh
    Video.hh
    VideoEncoder.hh
  )
endif()

set (ign_headers "" CACHE INTERNAL "Ignition headers" FORCE)
foreach (hdr ${headers})
  APPEND_TO_CACHED_STRING(ign_headers
    "Ignition headers" "#include <ignition/${IGN_PROJECT_NAME}/${hdr}>\n")
endforeach()

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/ign_auto_headers.hh.in
  ${CMAKE_CURRENT_BINARY_DIR}/${IGN_PROJECT_NAME}.hh)

ign_install_includes("${INCLUDE_INSTALL_DIR_POSTFIX}/ignition" ${CMAKE_CURRENT_BINARY_DIR}/${IGN_PROJECT_NAME}.hh)
ign_install_includes("${INCLUDE_INSTALL_DIR_POSTFIX}/ignition/${IGN_PROJECT_NAME}" ${headers})
