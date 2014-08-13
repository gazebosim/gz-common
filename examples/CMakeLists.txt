cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# Find the ignition-common library
find_package(ignition-common QUIET REQUIRED)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${IGNITION-COMMON_CXX_FLAGS}")
include_directories(${IGNITION-COMMON_INCLUDE_DIRS})
link_directories(${IGNITION-COMMON_LIBRARY_DIRS})

add_executable(assert_example assert_example.cc)
add_executable(exception_example exception_example.cc)
