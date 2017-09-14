#################################################
# Find tinyxml2. Only debian distributions package tinyxml with a pkg-config.

# Use pkg_check_modules to start
find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
  pkg_check_modules(TINYXML2 tinyxml2)
endif()

# If that failed, then fall back to manual detection (necessary for MacOS)
if(NOT TINYXML2_FOUND)

    find_path(TINYXML2_INCLUDE_DIRS tinyxml2.h ${TINYXML2_INCLUDE_DIRS} ENV CPATH)
    find_library(TINYXML2_LIBRARIES NAMES tinyxml2)
    set(TINYXML2_FOUND true)

    if(NOT TINYXML2_INCLUDE_DIRS)
      message(STATUS "Looking for tinyxml2 headers - not found")
      set(TINYXML2_FOUND false)
    endif()

    if(NOT TINYXML2_LIBRARIES)
      message (STATUS "Looking for tinyxml2 library - not found")
      set(TINYXML2_FOUND false)
    endif()

    if (NOT TINYXML2_LIBRARY_DIRS)
      message (STATUS "Looking for tinyxml2 library dirs - not found")
      set(TINYXML2_FOUND false)
    endif()

endif()
