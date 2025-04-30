if (TARGET ${PROJECT_LIBRARY_TARGET_NAME}-graphics)
  gz_get_libsources_and_unittests(sources gtest_sources)
  gz_add_component(geospatial
    SOURCES ${sources}
    DEPENDS_ON_COMPONENTS graphics
    GET_TARGET_NAME geospatial_target)

  target_link_libraries(${geospatial_target}
    PUBLIC
      ${PROJECT_LIBRARY_TARGET_NAME}-graphics
      gz-math::gz-math
      gz-utils::gz-utils
    PRIVATE
      ${GDAL_LIBRARY})

  target_include_directories(${geospatial_target}
    PRIVATE
      ${GDAL_INCLUDE_DIR})

  gz_build_tests(TYPE UNIT SOURCES ${gtest_sources}
    LIB_DEPS
      ${geospatial_target}
      gz-common-testing
  )
else()
  message(WARNING "Skipping component [geospatial]: Missing component [graphics].\n    ^~~~~ Set SKIP_geospatial=true in cmake to suppress this warning.")
endif()
