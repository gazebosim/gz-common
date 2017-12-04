# FIXME: Some classes fail to build on Windows
if(WIN32)
  set(exclude_headers
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

ign_install_all_headers(EXCLUDE ${exclude_headers})
