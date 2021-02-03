option(IGN_COMMON_BUILD_HW_VIDEO
  "Build support for HW-accelerated video encoding" ON)

ign_get_libsources_and_unittests(sources gtest_sources)

if(NOT IGN_COMMON_BUILD_HW_VIDEO)
  list(REMOVE_ITEM sources HWEncoder.cc)
endif()

ign_add_component(av SOURCES ${sources} GET_TARGET_NAME av_target)

target_link_libraries(${av_target}
  PUBLIC
    SWSCALE::SWSCALE
    AVDEVICE::AVDEVICE
    AVFORMAT::AVFORMAT
    AVCODEC::AVCODEC
    AVUTIL::AVUTIL)

if(IGN_COMMON_BUILD_HW_VIDEO)
  target_compile_definitions(${av_target} PRIVATE IGN_COMMON_BUILD_HW_VIDEO)
endif()

ign_build_tests(TYPE UNIT SOURCES ${gtest_sources}
  LIB_DEPS
    ${av_target}
    ignition-cmake${IGN_CMAKE_VER}::utilities
)

