add_library(GzBadPluginAlign         SHARED BadPluginAlign.cc)
target_link_libraries(GzBadPluginAlign PRIVATE ${PROJECT_LIBRARY_TARGET_NAME})
add_library(GzBadPluginAPIVersionNew SHARED BadPluginAPIVersionNew.cc)
target_link_libraries(GzBadPluginAPIVersionNew PRIVATE ${PROJECT_LIBRARY_TARGET_NAME})
add_library(GzBadPluginAPIVersionOld SHARED BadPluginAPIVersionOld.cc)
target_link_libraries(GzBadPluginAPIVersionOld PRIVATE ${PROJECT_LIBRARY_TARGET_NAME})
add_library(GzBadPluginSize          SHARED BadPluginSize.cc)
target_link_libraries(GzBadPluginSize PRIVATE ${PROJECT_LIBRARY_TARGET_NAME})
add_library(GzDummyPlugins           SHARED DummyPlugins.cc)
target_link_libraries(GzDummyPlugins PRIVATE ${PROJECT_LIBRARY_TARGET_NAME})

# Create a variable for the name of the header which will contain the dummy plugin path.
# This variable gets put in the cache so that it is available at generation time.
set(GZ_DUMMY_PLUGIN_PATH_HEADER "DummyPluginsPath.h" CACHE INTERNAL "Name of header which contains DummyPlugin library path")

if(WIN32)
  get_target_property(GzDummyPluginsLocation GzDummyPlugins RUNTIME_OUTPUT_DIRECTORY)
else()
  get_target_property(GzDummyPluginsLocation GzDummyPlugins LIBRARY_OUTPUT_DIRECTORY)
endif()
set(header_text "#define GZ_DUMMY_PLUGIN_PATH \"${GzDummyPluginsLocation}\"")

# Pipe the header text into a header that will be placed in the project binary directory.
# We need to use this approach instead of the usual configure_file, because we required a
# generator expression in order to locate the GzDummyPlugins library on Windows. Generator
# expressions can only be evaluated during generation (as opposed to during configuration),
# so we must use add_custom_command in order to produce this header.
add_custom_target(GzDummyPluginsPathHeader)
if(WIN32)
  # Including VERBATIM causes the wrong behavior on Windows, so we leave it out of the 
  # custom command when compiling on Windows.
  add_custom_command(TARGET GzDummyPluginsPathHeader
        PRE_BUILD
        COMMAND echo
        ARGS "${header_text}" > ${PROJECT_BINARY_DIR}/${GZ_DUMMY_PLUGIN_PATH_HEADER})
else()
  add_custom_command(TARGET GzDummyPluginsPathHeader
      PRE_BUILD
      COMMAND echo
      ARGS "${header_text}" > ${PROJECT_BINARY_DIR}/${GZ_DUMMY_PLUGIN_PATH_HEADER}
      VERBATIM)
endif()
add_dependencies(GzDummyPlugins GzDummyPluginsPathHeader)
