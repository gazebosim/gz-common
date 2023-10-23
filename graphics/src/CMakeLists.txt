gz_get_libsources_and_unittests(sources gtest_sources)

gz_add_component(graphics SOURCES ${sources} GET_TARGET_NAME graphics_target)

target_link_libraries(${graphics_target}
  PUBLIC
    gz-math${GZ_MATH_VER}::gz-math${GZ_MATH_VER}
    gz-utils${GZ_UTILS_VER}::gz-utils${GZ_UTILS_VER}
  PRIVATE
    ${GzAssimp_LIBRARIES}
    GTS::GTS
    FreeImage::FreeImage)

gz_build_tests(
  TYPE UNIT
  SOURCES ${gtest_sources}
  LIB_DEPS
    ${graphics_target}
    gz-common${GZ_COMMON_VER}-testing
)

# Assimp doesn't offer preprocessor version, use cmake to set a compatibility
# mode for versions below 5.2.0
if(${GzAssimp_VERSION} STRLESS "5.2.0")
  message("Warning, assimp below 5.2.0 detected, setting compatibility mode")
  target_compile_definitions(${graphics_target} PRIVATE GZ_ASSIMP_PRE_5_2_0)
  if(TARGET UNIT_AssimpLoader_TEST)
    target_compile_definitions(UNIT_AssimpLoader_TEST PRIVATE GZ_ASSIMP_PRE_5_2_0)
  endif()
endif()


# If we are using an external copy of tinyxml2, add its imported target
target_link_libraries(${graphics_target}
  PRIVATE
    TINYXML2::TINYXML2)

# The collada exporter test uses tinyxml2, so we must link it if we're using
# an external copy. The graphics target considers tinyxml2 to be a private
# dependency, so it will not automatically get linked to this test.
if(TARGET UNIT_ColladaExporter_TEST)
  target_link_libraries(UNIT_ColladaExporter_TEST TINYXML2::TINYXML2)
endif()
