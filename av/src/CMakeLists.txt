ign_get_libsources_and_unittests(sources gtest_sources)

# FIXME: This class does not currently work
list(REMOVE_ITEM sources AudioDecoder.cc)
list(REMOVE_ITEM gtest_sources AudioDecoder_TEST.cc)

ign_add_component(av SOURCES ${sources} GET_TARGET_NAME av_target)

target_link_libraries(${av_target}
  PUBLIC
    SWSCALE::SWSCALE
    AVDEVICE::AVDEVICE
    AVFORMAT::AVFORMAT
    AVCODEC::AVCODEC
    AVUTIL::AVUTIL)

ign_build_tests(TYPE UNIT SOURCES ${gtest_sources}
  LIB_DEPS ${av_target})
