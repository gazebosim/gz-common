include (IgnUtils)

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

set(ign_headers "")
foreach(hdr ${headers})
  list(APPEND ign_headers "#include <ignition/${IGN_DESIGNATION}/${hdr}>\n")
endforeach()

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/ign_auto_headers.hh.in
  ${CMAKE_CURRENT_BINARY_DIR}/${IGN_DESIGNATION}.hh)

ign_install_includes("${IGN_INCLUDE_INSTALL_DIR_POSTFIX}/ignition" ${CMAKE_CURRENT_BINARY_DIR}/${IGN_DESIGNATION}.hh)
ign_install_includes("${IGN_INCLUDE_INSTALL_DIR_POSTFIX}/ignition/${IGN_DESIGNATION}" ${headers})


########################################
# Write the config.h file
configure_file("config.hh.in"
  "${PROJECT_BINARY_DIR}/include/ignition/${IGN_DESIGNATION}/config.hh")

ign_install_includes(
  "${IGN_INCLUDE_INSTALL_DIR_POSTFIX}/ignition/${IGN_DESIGNATION}"
  "${PROJECT_BINARY_DIR}/include/ignition/${IGN_DESIGNATION}/config.hh")
