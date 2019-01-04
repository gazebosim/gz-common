set(
  Remotery_SRC
  ./Remotery/lib/Remotery.c
  ./Remotery/lib/Remotery.h
)

if(APPLE)
  set(
    Remotery_SRC
    ${Remotery_SRC}
    ./Remotery/lib/RemoteryMetal.mm
  )
endif()

set(RMT_ENABLED 1)
set(RMT_USE_TINYCRT 0)
set(RMT_USE_CUDA 0)
set(RMT_USE_D3D11 0)
set(RMT_USE_OPENGL 0)
set(RMT_USE_METAL 0)

if(UNIX OR WIN32)
  set(RMT_USE_OPENGL 1)
endif()

if(WIN32)
  set(RMT_USE_D3D11 1)
endif()

if(APPLE)
  set (RMT_USE_METAL 1)
endif()

configure_file(RemoteryConfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/RemoteryConfig.h)

ign_get_libsources_and_unittests(sources gtest_sources)

list(APPEND sources ${Remotery_SRC})

ign_add_component(profiler SOURCES ${sources} GET_TARGET_NAME profiler_target)

target_compile_definitions(${profiler_target} PRIVATE "IGN_PROFILER_REMOTERY=1")

target_include_directories(${profiler_target} PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Remotery/lib>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)

if (UNIX)
  target_link_libraries(${profiler_target} PUBLIC pthread)
endif()

ign_build_tests(TYPE UNIT SOURCES ${gtest_sources}
  LIB_DEPS ${profiler_target}
  TEST_LIST profiler_tests)

foreach(profiler_test ${profiler_tests})
  target_compile_definitions(${profiler_test} PUBLIC "IGN_ENABLE_PROFILER=1")
endforeach()

