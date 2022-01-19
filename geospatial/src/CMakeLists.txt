
ign_get_libsources_and_unittests(sources gtest_sources)

ign_add_component(geospatial
  SOURCES ${sources}
  DEPENDS_ON_COMPONENTS graphics
  GET_TARGET_NAME geospatial_target)

target_link_libraries(${geospatial_target}
  PUBLIC
    ${PROJECT_LIBRARY_TARGET_NAME}-graphics
    ignition-math${IGN_MATH_VER}::ignition-math${IGN_MATH_VER}
    ignition-utils${IGN_UTILS_VER}::ignition-utils${IGN_UTILS_VER}
  PRIVATE
    ${GDAL_LIBRARY})

target_include_directories(${geospatial_target}
  PRIVATE
    ${GDAL_INCLUDE_DIR})

ign_build_tests(TYPE UNIT SOURCES ${gtest_sources}
  LIB_DEPS ${geospatial_target})
