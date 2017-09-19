#.rst
# IgnConfigureProject
# -------------------
#
# ign_configure_project(ign_designation major_version minor_version patch_number)
#
# Sets up an ignition library project. Note that ign_designation should only be
# the second part of the library name (i.e. it should exclude the "ignition-"
# prefix).
#
#===============================================================================
# Copyright (C) 2017 Open Source Robotics Foundation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#################################################
# Initialize the ignition project
macro(ign_configure_project designation major_version minor_version patch_number)

  #============================================================================
  # Initiate project
  #============================================================================
  project(ignition-${designation}${major_version}
    VERSION ${major_version}.${minor_version}.${patch_number})

  # Note: The following are automatically defined by project(~) in cmake v3:
  # PROJECT_VERSION_MAJOR
  # PROJECT_VERSION_MINOR
  # PROJECT_VERSION_PATCH

  #============================================================================
  # Set project variables
  #============================================================================
  set(IGN_DESIGNATION "${designation}")

  set(PROJECT_NAME_NO_VERSION "ignition-${IGN_DESIGNATION}")
  string(TOLOWER ${PROJECT_NAME_NO_VERSION} PROJECT_NAME_NO_VERSION_LOWER)
  string(TOUPPER ${PROJECT_NAME_NO_VERSION} PROJECT_NAME_NO_VERSION_UPPER)
  string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWER)
  string(TOUPPER ${PROJECT_NAME} PROJECT_NAME_UPPER)

  set(PROJECT_EXPORT_NAME ${PROJECT_NAME_LOWER})
  set(PROJECT_LIBRARY_TARGET_NAME ${PROJECT_NAME_LOWER})

  set(PROJECT_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR})
  set(PROJECT_VERSION_FULL
    ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})

  message(STATUS "${PROJECT_NAME} version ${PROJECT_VERSION_FULL}")

  #============================================================================
  # Identify the operating system
  ign_check_os()

  #============================================================================
  # Create package information
  ign_setup_packages()

  #============================================================================
  # Create documentation
  ign_create_docs()

  #============================================================================
  # Collect compiler features for C++11. Creates a variable called
  #   CXX_11_COMPILE_FEATURES
  ign_collect_compile_features()

  #============================================================================
  # Initialize build errors/warnings
  # NOTE: We no longer use CACHE for these variables because it was set to
  # "INTERNAL", making it unnecessary to cache them. As long as this macro is
  # called from the top-level scope, these variables will effectively be global,
  # even without putting them in the cache. If this macro is not being called
  # from the top-level scope, then it is being used incorrectly.
  set(build_errors "")
  set(build_warnings "")


  #============================================================================
  # Initialize the list of <PROJECT_NAME>-config.cmake dependencies
  set(PROJECT_CMAKE_DEPENDENCIES)

  # Initialize the list of <PROJECT_NAME>.pc Requires
  set(PROJECT_PKGCONFIG_REQUIRES)

  # Initialize the list of <PROJECT_NAME>.pc Requires.private
  set(PROJECT_PKCONFIG_REQUIRES_PRIVATE)

  # Initialize the list of <PROJECT_NAME>.pc Libs
  set(PROJECT_PKGCONFIG_LIBS)

  # Initialize the list of <PROJECT_NAME>.pc Libs.private
  set(PROJECT_PKGCONFIG_LIBS_PRIVATE)

endmacro()

#################################################
# Collect the compiler features
macro(ign_collect_compile_features)
  # It's unfortunate that CMake does not provide this automatically

  set(CXX_PROPOSED_COMPILE_FEATURES
    cxx_uniform_initialization
  )

  set(CXX_1z_COMPILE_FEATURES
    cxx_aggregate_default_initializers
  )

  set(CXX_14_COMPILE_FEATURES
    cxx_attribute_deprecated
    cxx_binary_literals
    cxx_contextual_conversions
    cxx_decltype_auto
    cxx_digit_separators
    cxx_generic_lambdas
    cxx_lambda_init_captures
    cxx_relaxed_constexpr
    cxx_return_type_deduction
    cxx_variable_templates
  )

  set(CXX_11_COMPILE_FEATURES
    cxx_alias_templates
    cxx_alignas
    cxx_alignof
    cxx_attributes
    cxx_auto_type
    cxx_constexpr
    cxx_decltype_incomplete_return_types
    cxx_decltype
    cxx_default_function_template_args
    cxx_defaulted_functions
    cxx_defaulted_move_initializers
    cxx_delegating_constructors
    cxx_deleted_functions
    cxx_enum_forward_declarations
    cxx_explicit_conversions
    cxx_extended_friend_declarations
    cxx_extern_templates
    cxx_final
    cxx_func_identifier
    cxx_generalized_initializers
    cxx_inheriting_constructors
    cxx_inline_namespaces
    cxx_lambdas
    cxx_local_type_template_args
    cxx_long_long_type
    cxx_noexcept
    cxx_nonstatic_member_init
    cxx_nullptr
    cxx_override
    cxx_range_for
    cxx_raw_string_literals
    cxx_reference_qualified_functions
    cxx_right_angle_brackets
    cxx_rvalue_references
    cxx_sizeof_member
    cxx_static_assert
    cxx_strong_enums
    cxx_thread_local
    cxx_trailing_return_types
    cxx_unicode_literals
    cxx_unrestricted_unions
    cxx_user_literals
    cxx_variadic_macros
    cxx_variadic_templates
    cxx_template_template_parameters
  )

endmacro()

#################################################
# Check the OS type.
macro(ign_check_os)

  # CMake does not distinguish Linux from other Unices.
  string(REGEX MATCH "Linux" PLAYER_OS_LINUX ${CMAKE_SYSTEM_NAME})
  # Nor *BSD
  string(REGEX MATCH "BSD" PLAYER_OS_BSD ${CMAKE_SYSTEM_NAME})
  # Or Solaris. I'm seeing a trend, here
  string(REGEX MATCH "SunOS" PLAYER_OS_SOLARIS ${CMAKE_SYSTEM_NAME})

  # Windows is easy (for once)
  if(WIN32)
    set(PLAYER_OS_WIN TRUE BOOL INTERNAL)
  endif()

  # Check if it's an Apple OS
  if(APPLE)
    # Check if it's OS X or another MacOS (that's got to be pretty unlikely)
    string(REGEX MATCH "Darwin" PLAYER_OS_OSX ${CMAKE_SYSTEM_NAME})
    if(NOT PLAYER_OS_OSX)
      set(PLAYER_OS_MACOS TRUE BOOL INTERNAL)
    endif()
  endif()

  # QNX
  if(QNXNTO)
    set(PLAYER_OS_QNX TRUE BOOL INTERNAL)
  endif()

  if(PLAYER_OS_LINUX)
    message(STATUS "Operating system is Linux")
  elseif(PLAYER_OS_BSD)
    message(STATUS "Operating system is BSD")
  elseif(PLAYER_OS_WIN)
    message(STATUS "Operating system is Windows")
  elseif(PLAYER_OS_OSX)
    message(STATUS "Operating system is Apple MacOS X")
  elseif(PLAYER_OS_MACOS)
    message(STATUS "Operating system is Apple MacOS (not OS X)")
  elseif(PLAYER_OS_QNX)
    message(STATUS "Operating system is QNX")
  elseif(PLAYER_OS_SOLARIS)
    message(STATUS "Operating system is Solaris")
  else(PLAYER_OS_LINUX)
    message(STATUS "Operating system is generic Unix")
  endif()

  #################################################
  # Check for non-case-sensitive filesystems
  execute_process(COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tools/case_sensitive_filesystem
                  RESULT_VARIABLE FILESYSTEM_CASE_SENSITIVE_RETURN)
  if (${FILESYSTEM_CASE_SENSITIVE_RETURN} EQUAL 0)
    set(FILESYSTEM_CASE_SENSITIVE TRUE)
  else()
    set(FILESYSTEM_CASE_SENSITIVE FALSE)
  endif()

endmacro()
