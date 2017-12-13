add_library(IGNBadPluginAlign         SHARED BadPluginAlign.cc)
add_library(IGNBadPluginAPIVersionNew SHARED BadPluginAPIVersionNew.cc)
add_library(IGNBadPluginAPIVersionOld SHARED BadPluginAPIVersionOld.cc)
add_library(IGNBadPluginSize          SHARED BadPluginSize.cc)
add_library(IGNDummyPlugins SHARED DummyPlugins.cc)

# Create a variable for the name of the header which will contain the dummy plugin path.
# This variable gets put in the cache so that it is available at generation time.
set(IGN_DUMMY_PLUGIN_PATH_HEADER "DummyPluginsPath.h" CACHE INTERNAL "Name of header which contains DummyPlugin library path")

set(header_text "#define IGN_DUMMY_PLUGIN_PATH \"$<TARGET_FILE_DIR:IGNDummyPlugins>\"")

# Pipe the header text into a header that will be placed in the project binary directory.
# We need to use this approach instead of the usual configure_file, because we required a
# generator expression in order to locate the IGNDummyPlugins library on Windows. Generator
# expressions can only be evaluated during generation (as opposed to during configuration),
# so we must use add_custom_command in order to produce this header.
if(WIN32)
  # Including VERBATIM causes the wrong behavior on Windows, so we leave it out of the 
  # custom command when compiling on Windows.
  add_custom_command(TARGET IGNDummyPlugins
        PRE_BUILD
        COMMAND echo
        ARGS "${header_text}" > ${PROJECT_BINARY_DIR}/${IGN_DUMMY_PLUGIN_PATH_HEADER})
else()
  add_custom_command(TARGET IGNDummyPlugins
      PRE_BUILD
      COMMAND echo
      ARGS "${header_text}" > ${PROJECT_BINARY_DIR}/${IGN_DUMMY_PLUGIN_PATH_HEADER}
      VERBATIM)
endif()