set(TEST_TYPE "PERFORMANCE")

set(tests
  plugin_specialization.cc
)

link_directories(${PROJECT_BINARY_DIR}/test)

ign_build_tests(${tests})
