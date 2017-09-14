
########################################
# Find avcodec
pkg_check_modules(AVCODEC libavcodec)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  AVCODEC
  REQUIRED_VARS AVCODEC_FOUND)
