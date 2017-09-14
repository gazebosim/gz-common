
########################################
# Find AV device.
set(av_major ${AVDEVICE_FIND_VERSION_MAJOR})
set(av_minor ${AVDEVICE_FIND_VERSION_MINOR})
set(av_patch ${AVDEVICE_FIND_VERSION_PATCH})

pkg_check_modules(AVDEVICE libavdevice>=${av_major}.${av_minor}.${av_patch})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  AVDEVICE
  REQUIRED_VARS AVDEVICE_FOUND)
