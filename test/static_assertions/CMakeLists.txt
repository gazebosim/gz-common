set(static_assert_tests
  plugin_bad_const_assignment
  plugin_bad_const_construct
  plugin_no_base_class
  # TODO Fix FQN name check
  # plugin_not_fully_qualified_class
  # plugin_not_fully_qualified_base
  plugin_not_global_ns
  plugin_wrong_base_class
  )

if (UNIX)
  set(flag -Wno-unused-private-field)
  CHECK_CXX_COMPILER_FLAG(${flag} Rflag)
  # Make tests that try to build these files
  foreach (test ${static_assert_tests})
    add_executable(${test} ${test}.cc)
    set_target_properties(${test} PROPERTIES
      EXCLUDE_FROM_ALL TRUE
      EXCLUDE_FROM_DEFAULT_BUILD TRUE)
    if (Rflag)
      target_compile_options(${test} PRIVATE ${flag})
    endif()
    add_test(NAME STATIC_ASSERT_${test}
      COMMAND ${CMAKE_SOURCE_DIR}/test/static_assertions/testrunner.bash ${CMAKE_COMMAND} --build . --target ${test} --config $<CONFIGURATION>
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
  endforeach()

  # Tests pass if the output contains the expected error

  set_tests_properties(STATIC_ASSERT_plugin_bad_const_assignment PROPERTIES
    PASS_REGULAR_EXPRESSION "The requested PluginPtr cast would discard const qualifiers")

  set_tests_properties(STATIC_ASSERT_plugin_bad_const_construct PROPERTIES
    PASS_REGULAR_EXPRESSION "The requested PluginPtr cast would discard const qualifiers")

  set_tests_properties(STATIC_ASSERT_plugin_no_base_class PROPERTIES
    PASS_REGULAR_EXPRESSION "is not a base class of")

  set_tests_properties(STATIC_ASSERT_plugin_not_global_ns PROPERTIES
    PASS_REGULAR_EXPRESSION "IGN_macro_must_be_used_in_global_namespace")

  set_tests_properties(STATIC_ASSERT_plugin_wrong_base_class PROPERTIES
    PASS_REGULAR_EXPRESSION "is not a base class of")
endif()
