set(TEST_TYPE "BENCHMARK")
find_package(benchmark)
include(GzBenchmark OPTIONAL RESULT_VARIABLE GzBenchmark_FOUND)

if (GzBenchmark_FOUND)
  set(tests
    MeshManager.cc
  )

  gz_add_benchmarks(SOURCES ${tests})

  if (TARGET BENCHMARK_MeshManager)
    target_link_libraries(BENCHMARK_MeshManager
      ${PROJECT_LIBRARY_TARGET_NAME}-graphics
      ${PROJECT_LIBRARY_TARGET_NAME}-testing
      gtest
    )
    target_compile_definitions(BENCHMARK_MeshManager PRIVATE
      "TESTING_PROJECT_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\"")
  endif()

  if (benchmark_VERSION VERSION_LESS "1.9.5")
    add_compile_definitions(BENCHMARK_INTERNAL)
  endif()

endif()
