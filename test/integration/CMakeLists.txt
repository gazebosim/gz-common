# This is to make test_config.h visible
include_directories("${CMAKE_BINARY_DIR}")

ign_get_sources(tests)

# FIXME the mesh test does not work
list(REMOVE_ITEM tests mesh.cc)

ign_build_tests(
  TYPE INTEGRATION
  SOURCES ${tests})

if(TARGET INTEGRATION_plugin)
  # We add this dependency to make sure that DummyPlugins gets generated
  # before INTEGRATION_plugin so that its auto-generated header is available.
  # We do not want to link INTEGRATION_plugin to the DummyPlugins library.
  add_dependencies(INTEGRATION_plugin IGNDummyPlugins)
endif()
