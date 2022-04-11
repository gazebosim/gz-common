set(sources
  BazelTestPaths.cc
  CMakeTestPaths.cc
  TestPaths.cc
  Utils.cc
)

set(test_sources
  AutoLogFixture_TEST.cc
  CMakeTestPaths_TEST.cc
  Utils_TEST.cc
)

ign_add_component(testing SOURCES ${sources} GET_TARGET_NAME testing_target)

ign_build_tests(TYPE UNIT SOURCES ${test_sources}
  LIB_DEPS ${testing_target})
