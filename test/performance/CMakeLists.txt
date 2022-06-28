gz_get_sources(tests)

# plugin_specialization test causes lcov to hang
# see gz-cmake issue 25
if("${CMAKE_BUILD_TYPE_UPPERCASE}" STREQUAL "COVERAGE")
  list(REMOVE_ITEM tests
    plugin_specialization.cc)
endif()

gz_build_tests(
  TYPE PERFORMANCE
  SOURCES ${tests})

if(TARGET PERFORMANCE_plugin_specialization)
  # We add this dependency to make sure that DummyPlugins gets generated
  # before PERFORMANCE_plugin_specialization so that its auto-generated header is available.
  add_dependencies(PERFORMANCE_plugin_specialization GzDummyPlugins)
  target_include_directories(PERFORMANCE_plugin_specialization PRIVATE ${PROJECT_SOURCE_DIR}/test)
endif()
