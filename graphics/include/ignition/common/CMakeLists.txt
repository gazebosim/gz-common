# FIXME: Some classes fail to build on Windows
if(WIN32)
  set(exclude_headers
    graphics/GTSMeshUtils.hh
    graphics/Image.hh
    graphics/ImageHeightmap.hh
    graphics/MeshCSG.hh
  )
endif()

ign_install_all_headers(COMPONENT graphics EXCLUDE_FILES ${exclude_headers})
