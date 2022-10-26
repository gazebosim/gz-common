option(IGN_PROFILER_REMOTERY "Enable the Remotery profiler" TRUE)

set(
  PROFILER_SRCS
  Profiler.cc
)

set(
  PROFILER_TESTS
  Profiler_Disabled_TEST.cc
)

if(NOT WIN32)
  list(APPEND PROFILER_TESTS Profiler_Error_TEST.cc)
endif()

if(IGN_PROFILER_REMOTERY)
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
    find_library(FOUNDATION Foundation
      HINTS /System/Library/Frameworks
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

  list(APPEND PROFILER_SRCS ${Remotery_SRC} RemoteryProfilerImpl.cc)
  list(APPEND PROFILER_TESTS Profiler_Remotery_TEST.cc)
endif()

ign_add_component(profiler SOURCES ${PROFILER_SRCS} GET_TARGET_NAME profiler_target)

# Always enable profiler so that it's built, but make it
# private so that downstream users can still disable profiling
target_compile_definitions(${profiler_target} PRIVATE "IGN_PROFILER_ENABLE=1")
target_compile_definitions(${profiler_target} PRIVATE "RMT_USE_METAL=${RMT_USE_METAL}")

if(IGN_PROFILER_REMOTERY)
  target_compile_definitions(${profiler_target} PRIVATE "IGN_PROFILER_REMOTERY=1")

  target_include_directories(
    ${profiler_target}
    PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Remotery/lib>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  )

  if (UNIX)
    target_link_libraries(${profiler_target} PUBLIC pthread)
  endif()

  if(APPLE)
    target_link_libraries(${profiler_target} PUBLIC ${FOUNDATION})
  endif()
endif()

ign_build_tests(TYPE UNIT SOURCES ${PROFILER_TESTS}
  LIB_DEPS ${profiler_target}
  TEST_LIST profiler_tests)

if(TARGET UNIT_Profiler_Remotery_TEST)
  target_compile_definitions(UNIT_Profiler_Remotery_TEST
    PUBLIC "IGN_PROFILER_ENABLE=1")
endif()

if(IGN_PROFILER_REMOTERY)
  set(IGN_PROFILER_VIS_PATH ${IGN_DATA_INSTALL_DIR}/profiler_vis)

  configure_file(Remotery/ign_remotery_vis.in
    ${CMAKE_CURRENT_BINARY_DIR}/ign_remotery_vis)

  install(PROGRAMS
    ${CMAKE_CURRENT_BINARY_DIR}/ign_remotery_vis
    DESTINATION bin)

  install(DIRECTORY Remotery/vis/
          DESTINATION ${IGN_DATA_INSTALL_DIR}/profiler_vis)
endif()
