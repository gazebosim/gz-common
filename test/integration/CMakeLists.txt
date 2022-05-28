ign_get_sources(tests)

if (SKIP_av OR INTERNAL_SKIP_av)
  list(REMOVE_ITEM tests encoder_timing.cc)
  list(REMOVE_ITEM tests video_encoder.cc)
endif()

ign_build_tests(
  TYPE INTEGRATION
  SOURCES ${tests}
  LIB_DEPS ignition-common${IGN_COMMON_VER}-testing
)

if(TARGET INTEGRATION_plugin)
  # We add this dependency to make sure that DummyPlugins gets generated
  # before INTEGRATION_plugin so that its auto-generated header is available.
  # We do not want to link INTEGRATION_plugin to the DummyPlugins library.
  add_dependencies(INTEGRATION_plugin GzDummyPlugins)
endif()

if(TARGET INTEGRATION_mesh)
  target_link_libraries(INTEGRATION_mesh ${PROJECT_LIBRARY_TARGET_NAME}-graphics)
endif()

if(TARGET INTEGRATION_encoder_timing)
  target_link_libraries(INTEGRATION_encoder_timing ${PROJECT_LIBRARY_TARGET_NAME}-av)
endif()

if(TARGET INTEGRATION_video_encoder)
  target_link_libraries(INTEGRATION_video_encoder ${PROJECT_LIBRARY_TARGET_NAME}-av)
endif()
