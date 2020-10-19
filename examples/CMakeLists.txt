cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# Find the ignition-common library
find_package(ignition-common4 QUIET REQUIRED COMPONENTS events profiler)

add_executable(assert_example assert_example.cc)
target_link_libraries(assert_example ignition-common4::core)

add_executable(console_example console.cc)
target_link_libraries(console_example ignition-common4::core)

add_executable(events_example events.cc)
target_link_libraries(events_example ignition-common4::events)

add_executable(profiler_example profiler.cc)
target_link_libraries(profiler_example ignition-common4::profiler)
target_compile_definitions(profiler_example PUBLIC "IGN_PROFILER_ENABLE=1")
