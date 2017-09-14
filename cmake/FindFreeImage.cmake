find_package(PkgConfig QUIET)
include(IgnUtils)

########################################
# Find packages

# Grab the version numbers requested by the call to find_package(~)
set(major_version ${FreeImage_FIND_VERSION_MAJOR})
set(minor_version ${FreeImage_FIND_VERSION_MINOR})

# Set the full version number
set(full_version ${major_version}.${minor_version})

if(PKG_CONFIG_FOUND)
  pkg_check_modules(FreeImage FreeImage>=${full_version})
  if (NOT FreeImage_FOUND)
    message (STATUS "FreeImage.pc not found, we will search for FreeImage_INCLUDE_DIRS and FreeImage_LIBRARIES")
  endif (NOT FreeImage_FOUND)
endif (PKG_CONFIG_FOUND)

if(NOT FreeImage_FOUND)
  find_path(FreeImage_INCLUDE_DIRS FreeImage.h)
  if(NOT FreeImage_INCLUDE_DIRS)
    message (STATUS "Looking for FreeImage.h - not found")
    ign_build_error("Missing: Unable to find FreeImage.h")
  else(NOT FreeImage_INCLUDE_DIRS)
    # Check the FreeImage header for the right version
    set(testFreeImageSource ${CMAKE_CURRENT_BINARY_DIR}/CMakeTmp/test_freeimage.cc)
    set(FreeImage_test_output "")
    set(FreeImage_compile_output "")
    file(WRITE ${testFreeImageSource}
      "#include <FreeImage.h>\nint main () { if (FREEIMAGE_MAJOR_VERSION >= ${major_version} && FREEIMAGE_MINOR_VERSION >= ${minor_version}) return 1; else return 0;} \n")

    try_run(FREEIMAGE_RUNS
            FREEIMAGE_COMPILES
            ${CMAKE_CURRENT_BINARY_DIR}
            ${testFreeImageSource}
            CMAKE_FLAGS -DINCLUDE_DIRECTORIES=${FreeImage_INCLUDE_DIRS}
            RUN_OUTPUT_VARIABLE FreeImage_test_output
            COMPILE_OUTPUT_VARIABLE FreeImage_compile_output)

    if(NOT FREEIMAGE_COMPILES)
      ign_build_error("FreeImage test failed to compile - This may indicate a build system bug")
      return()
    endif (NOT FREEIMAGE_COMPILES)

    if(NOT FREEIMAGE_RUNS)
      ign_build_error("Invalid FreeImage Version. Requires ${major_version}.${minor_version}")
    endif(NOT FREEIMAGE_RUNS)
  endif(NOT FreeImage_INCLUDE_DIRS)

  find_library(FreeImage_LIBRARIES freeimage)
  if(FreeImage_LIBRARIES)
    set(FreeImage_FOUND true)
  else()
    set("Looking for libfreeimage - not found")
    ign_build_error("Missing: Unable to find libfreeimage")
  endif(FreeImage_LIBRARIES)
endif(NOT FreeImage_FOUND)
