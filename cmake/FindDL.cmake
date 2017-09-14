
########################################
# Find libdl

# If we cannot find the header or the library, we will switch this to false
set(DL_FOUND true)

# Search for the header
find_path(DL_INCLUDE_DIRS dlfcn.h)
if(DL_INCLUDE_DIRS)
  message (STATUS "Looking for dlfcn.h - found")
else(DL_INCLUDE_DIRS)
  message(STATUS "Looking for dlfcn.h - not found")
  set(DL_FOUND false)
endif()

# Search for the library
find_library(DL_LIBRARIES dl)
if(DL_LIBRARIES)
  message(STATUS "Looking for libdl - found")
else(DL_LIBRARIES)
  message(STATUS "Looking for libdl - not found")
  set(DL_FOUND false)
endif()
