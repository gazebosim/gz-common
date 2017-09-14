
########################################
# Find avutil
pkg_check_modules(AVUTIL libavutil)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  AVUTIL
  REQUIRED_VARS AVUTIL_FOUND)
