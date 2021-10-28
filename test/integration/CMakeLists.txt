# This is to make test_config.h visible
include_directories("${CMAKE_BINARY_DIR}")

ign_get_sources(tests)

# FIXME the mesh test does not work
list(REMOVE_ITEM tests mesh.cc)

if (SKIP_av OR INTERNAL_SKIP_av)
  list(REMOVE_ITEM tests encoder_timing.cc)
  list(REMOVE_ITEM tests video_encoder.cc)
endif()

ign_build_tests(
  TYPE INTEGRATION
  SOURCES ${tests}
)

if(TARGET INTEGRATION_plugin)
  # We add this dependency to make sure that DummyPlugins gets generated
  # before INTEGRATION_plugin so that its auto-generated header is available.
  # We do not want to link INTEGRATION_plugin to the DummyPlugins library.
  add_dependencies(INTEGRATION_plugin IGNDummyPlugins)
endif()

if(TARGET INTEGRATION_encoder_timing)
  target_link_libraries(INTEGRATION_encoder_timing ${PROJECT_LIBRARY_TARGET_NAME}-av)
endif()

if(TARGET INTEGRATION_video_encoder)
  target_link_libraries(INTEGRATION_video_encoder ${PROJECT_LIBRARY_TARGET_NAME}-av)
endif()
