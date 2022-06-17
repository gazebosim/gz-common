gz_get_libsources_and_unittests(sources gtest_sources)

gz_add_component(events SOURCES ${sources} GET_TARGET_NAME events_target)

target_link_libraries(${events_target}
  PUBLIC
    ignition-math${IGN_MATH_VER}::ignition-math${IGN_MATH_VER})

gz_build_tests(
  TYPE UNIT
  SOURCES ${gtest_sources}
  LIB_DEPS
    ${events_target}
    ignition-common${IGN_COMMON_VER}-testing
)
