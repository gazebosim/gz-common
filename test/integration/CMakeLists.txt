set(TEST_TYPE "INTEGRATION")

set(tests
  plugin.cc
)

include_directories(.)

link_directories(${PROJECT_BINARY_DIR}/test)

ign_build_tests(${tests})

# We add this dependency to make sure that DummyPlugins gets generated
# before INTEGRATION_plugin so that its auto-generated header is available.
# We do not want to link INTEGRATION_plugin to the DummyPlugins library.
add_dependencies(INTEGRATION_plugin IGNDummyPlugins)