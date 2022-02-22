set(sources
  TestPaths.cc
  CMakeTestPaths.cc
  BazelTestPaths.cc
)

set(test_sources
  CMakeTestPaths_TEST.cc
)

ign_add_component(testing SOURCES ${sources} GET_TARGET_NAME testing_target)

ign_build_tests(TYPE UNIT SOURCES ${test_sources}
  LIB_DEPS ${testing_target})
