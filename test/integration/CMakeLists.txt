set(TEST_TYPE "INTEGRATION")

set(tests
  plugin.cc
)

include_directories(.)

link_directories(${PROJECT_BINARY_DIR}/test)

ign_build_tests(${tests})
