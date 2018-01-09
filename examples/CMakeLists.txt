cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# Find the ignition-common library
find_package(ignition-common2 QUIET REQUIRED)

add_executable(assert_example assert_example.cc)
target_link_libraries(assert_example ${IGNITION-COMMON_LIBRARIES})
