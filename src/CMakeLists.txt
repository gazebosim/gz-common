include (${project_cmake_dir}/Utils.cmake)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set (sources
  Console.cc
  Time.cc
  Timer.cc
  Util.cc
)

set (gtest_sources
  Console_TEST.cc
  Exception_TEST.cc
  Time_TEST.cc
)

ign_add_library(ignition-common ${sources})
ign_install_library(ignition-common)

target_link_libraries(ignition-common ${Boost_LIBRARIES})

ign_build_tests(${gtest_sources})
