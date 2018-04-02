
ign_get_libsources_and_unittests(sources gtest_sources)


if(NOT USE_EXTERNAL_TINYXML2)
  # If we are using our internal copy of tinyxml2, then add its
  # source file.
  message(STATUS "Adding tinyxml2 source files")
  list(APPEND sources ${CMAKE_CURRENT_SOURCE_DIR}/tinyxml2/tinyxml2.cpp)
endif()


ign_add_component(graphics SOURCES ${sources} GET_TARGET_NAME graphics_target)

target_link_libraries(${graphics_target}
  PUBLIC
    ignition-math${IGN_MATH_VER}::ignition-math${IGN_MATH_VER}
  PRIVATE
    GTS::GTS
    FreeImage::FreeImage)

ign_build_tests(TYPE UNIT SOURCES ${gtest_sources}
  LIB_DEPS ${graphics_target})


if(USE_EXTERNAL_TINYXML2)

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

else()

  # Otherwise, add its source directory to our target's PRIVATE include
  # directories. We do not want this to be visible to consumers of
  # ignition-common.
  target_include_directories(${graphics_target}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tinyxml2)

  if(TARGET UNIT_ColladaExporter_TEST)

    # If we are using the internal copy of tinyxml2, then the collada exporter
    # test needs to be pointed to the internal tinyxml2 include directory.
    target_include_directories(UNIT_ColladaExporter_TEST PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tinyxml2)
    # We also need to add this as a source file to the collada exporter test's
    # target, because we do not export the tinyxml2 symbols that get generated
    # in the ignition-common library. Therefore, the collada exporter test
    # cannot link to those symbols and must generate its own.
    target_sources(UNIT_ColladaExporter_TEST PRIVATE tinyxml2/tinyxml2.cpp)

  endif()

endif()

# define of tinxml2 major version >= 6
# https://bitbucket.org/ignitionrobotics/ign-common/issues/28
if (NOT TINYXML2_VERSION VERSION_LESS "6.0.0")
  message(STATUS "TINYXML2_VERSION ${TINYXML2_VERSION} >= 6.0.0")
  target_compile_definitions(${graphics_target}
    PRIVATE "TINYXML2_MAJOR_VERSION_GE_6")
endif()
