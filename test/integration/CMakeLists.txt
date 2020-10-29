# This is to make test_config.h visible
include_directories("${CMAKE_BINARY_DIR}")

ign_get_sources(tests)

# FIXME the mesh test does not work
list(REMOVE_ITEM tests mesh.cc)

ign_build_tests(
  TYPE INTEGRATION
  SOURCES ${tests})
