add_subdirectory(Remotery)

ign_get_libsources_and_unittests(sources gtest_sources)

ign_add_component(profiler SOURCES ${sources} GET_TARGET_NAME profiler_target)

if(${IGN_PROFILER_REMOTERY})
  target_link_libraries(${profiler_target}
    PRIVATE ${Remotery_LIBRARIES})
endif()

ign_build_tests(TYPE UNIT SOURCES ${gtest_sources}
  LIB_DEPS ${profiler_target})

