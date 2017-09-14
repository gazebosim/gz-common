################################################################################
#ign_append_to_cached_string(_string _cacheDesc [items...])
# Appends items to a cached list.
macro(ign_append_to_cached_string _string _cacheDesc)
  foreach(newItem ${ARGN})
    set(${_string} "${${_string}} ${newItem}" CACHE INTERNAL ${_cacheDesc} FORCE)
  endforeach(newItem ${ARGN})
  #string(STRIP ${${_string}} ${_string})
endmacro(ign_append_to_cached_string)

################################################################################
# ign_append_to_cached_list (_list _cacheDesc [items...]
# Appends items to a cached list.
macro(ign_append_to_cached_list _list _cacheDesc)
  set(tempList ${${_list}})
  foreach(newItem ${ARGN})
    list(APPEND tempList ${newItem})
  endforeach(newItem ${newItem})
  set(${_list} ${tempList} CACHE INTERNAL ${_cacheDesc} FORCE)
endmacro(ign_append_to_cached_list)

#################################################
# Macro to turn a list into a string (why doesn't CMake have this built-in?)
macro(list_to_string _string _list)
    set(${_string})
    foreach(_item ${_list})
      set(${_string} "${${_string}} ${_item}")
    endforeach(_item)
    #string(STRIP ${${_string}} ${_string})
endmacro(list_to_string)

#################################################
# ign_find_package(<PACKAGE_NAME> [REQUIRED] [QUIET]
#                  [VERSION ver]
#                  [EXTRA_ARGS args]
#                  [PRETTY name]
#                  [PURPOSE "explanation for this dependency"])
#
# This is a wrapper for the standard cmake find_package which behaves according
# to the conventions of the ignition library. In particular, we do not quit
# immediately when a required package is missing. Instead, we check all
# dependencies and provide an overview of what is missing at the end of the
# configuration process. Descriptions of the function arguments are as follows:
#
# <PACKAGE_NAME>: The name of the package as it would normally be passed to
#                 find_package(~)
# [REQUIRED]: Optional. If provided, this will trigger an ignition build_error.
#             If not provided, this will trigger an ignition build_warning.
# [QUIET]: Optional. If provided, it will be passed forward to cmake's
#          find_package(~) command. This function will still print its normal
#          output.
# [VERSION]: Optional. Follow this argument with the major[.minor[.patch[.tweak]]]
#            version that you need for this package.
# [EXTRA_ARGS]: Optional. Additional args to pass forward to find_package(~)
# [PRETTY]: Optional. If provided, the string that follows will replace
#           <PACKAGE_NAME> when printing messages, warnings, or errors to the
#           terminal.
# [PURPOSE]: Optional. If provided, the string that follows will be appended to
#            the build_warning or build_error that this function produces when
#            the package could not be found.
macro(ign_find_package PACKAGE_NAME)

  #------------------------------------
  # Define the expected arguments
  set(options REQUIRED QUIET)
  set(oneValueArgs VERSION PRETTY PURPOSE EXTRA_ARGS)
  set(multiValueArgs) # We are not using multiValueArgs yet

  #------------------------------------
  # We need this to be a macro instead of a function so that the variables which
  # get set by find_package will make their way to the invoker, but this has the
  # unfortunate side effect that variables which get created for the optional
  # arguments will persist between invokations of this macro that occur in the
  # same scope. Therefore, we will manually unset those optional variables here
  # to keep make sure they don't pollute anything. If anyone has a better
  # solution for this, please share.
  foreach(arg ${options} ${oneValueArgs} ${multiValueArgs})
    unset(ign_find_package_${arg})
  endforeach()

  #------------------------------------
  # Parse the arguments
  cmake_parse_arguments(ign_find_package "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  #------------------------------------
  # Construct the arguments to pass to find_package
  if(ign_find_package_VERSION)
    list(APPEND ${PACKAGE_NAME}_find_package_args ${ign_find_package_VERSION})
  endif()

  if(ign_find_package_QUIET)
    list(APPEND ${PACKAGE_NAME}_find_package_args QUIET)
  endif()

  if(ign_find_package_EXTRA_ARGS)
    list(APPEND ${PACKAGE_NAME}_find_package_args ${ign_find_package_EXTRA_ARGS})
  endif()

  #------------------------------------
  # Figure out which name to print
  if(ign_find_package_PRETTY)
    set(${PACKAGE_NAME}_pretty ${ign_find_package_PRETTY})
  else()
    set(${PACKAGE_NAME}_pretty ${PACKAGE_NAME})
  endif()


  #------------------------------------
  # Call find_package with the provided arguments
  find_package(${PACKAGE_NAME} ${${PACKAGE_NAME}_find_package_args})
  if(${PACKAGE_NAME}_FOUND)
    message(STATUS "Looking for ${${PACKAGE_NAME}_pretty} - found\n")
  else()
    message(STATUS "Looking for ${${PACKAGE_NAME}_pretty} - not found\n")

    #------------------------------------
    # Construct the warning/error message to produce
    set(${PACKAGE_NAME}_msg "Missing: ${${PACKAGE_NAME}_pretty}")
    if(DEFINED ign_find_package_PURPOSE)
      set(${PACKAGE_NAME}_msg "${${PACKAGE_NAME}_msg} - ${ign_find_package_PURPOSE}")
    endif()

    if(ign_find_package_REQUIRED)
      ign_build_error(${${PACKAGE_NAME}_msg})
    else()
      ign_build_warning(${${PACKAGE_NAME}_msg})
    endif()
  endif()


endmacro()

#################################################
# ign_build_error macro
macro(ign_build_error)
  foreach(str ${ARGN})
    set(msg "\t${str}")
    list(APPEND build_errors ${msg})
  endforeach()
endmacro(ign_build_error)

#################################################
# ign_build_warning macro
macro(ign_build_warning)
  foreach(str ${ARGN})
    set(msg "\t${str}" )
    list(APPEND build_warnings ${msg})
  endforeach(str ${ARGN})
endmacro(ign_build_warning)

#################################################
macro(ign_add_library _name)
  set(LIBS_DESTINATION ${PROJECT_BINARY_DIR}/src)
  set_source_files_properties(${ARGN} PROPERTIES COMPILE_DEFINITIONS "BUILDING_DLL")
  add_library(${_name} SHARED ${ARGN})
  target_link_libraries (${_name} ${general_libraries})
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIBS_DESTINATION})
  if (MSVC)
    set_target_properties( ${_name} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${LIBS_DESTINATION})
    set_target_properties( ${_name} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${LIBS_DESTINATION})
    set_target_properties( ${_name} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${LIBS_DESTINATION})
  endif ( MSVC )
endmacro()

#################################################
macro(ign_add_static_library _name)
  add_library(${_name} STATIC ${ARGN})
  target_link_libraries(${_name} ${general_libraries})
endmacro()

#################################################
macro(ign_add_executable _name)
  add_executable(${_name} ${ARGN})
  target_link_libraries(${_name} ${general_libraries})
endmacro()


#################################################
macro(ign_install_includes _subdir)
  install(FILES ${ARGN}
    DESTINATION ${IGN_INCLUDE_INSTALL_DIR}/${_subdir} COMPONENT headers)
endmacro()

#################################################
macro(ign_install_library _name _exportName)
  set_target_properties(${_name} PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR} VERSION ${PROJECT_VERSION_FULL})
  install(TARGETS ${_name} EXPORT ${_exportName} DESTINATION ${IGN_LIB_INSTALL_DIR} COMPONENT shlib)
endmacro()

#################################################
macro(ign_install_executable _name )
  set_target_properties(${_name} PROPERTIES VERSION ${PROJECT_VERSION_FULL})
  install (TARGETS ${_name} DESTINATION ${IGN_BIN_INSTALL_DIR})
  manpage(${_name} 1)
endmacro()



# This should be migrated to more fine control solution based on set_property APPEND
# directories. It's present on cmake 2.8.8 while precise version is 2.8.7
link_directories(${PROJECT_BINARY_DIR}/test)
include_directories("${PROJECT_SOURCE_DIR}/test/gtest/include")

#################################################
# Enable tests compilation by default
if (NOT DEFINED ENABLE_TESTS_COMPILATION)
  set (ENABLE_TESTS_COMPILATION True)
endif()

#################################################
# Macro to setup supported compiler warnings
# Based on work of Florent Lamiraux, Thomas Moulard, JRL, CNRS/AIST.
include(CheckCXXCompilerFlag)

macro(ign_filter_valid_compiler_options var)
  # Store the current setting for CMAKE_REQUIRED_QUIET
  set(original_cmake_required_quiet ${CMAKE_REQUIRED_QUIET})

  # Make these tests quiet so they don't pollute the cmake output
  set(CMAKE_REQUIRED_QUIET true)

  foreach(flag ${ARGN})
    CHECK_CXX_COMPILER_FLAG(${flag} result${flag})
    if(result${flag})
      set(${var} "${${var}} ${flag}")
    endif()
  endforeach()

  # Restore the old setting for CMAKE_REQUIRED_QUIET
  set(CMAKE_REQUIRED_QUIET ${original_cmake_required_quiet})
endmacro()
