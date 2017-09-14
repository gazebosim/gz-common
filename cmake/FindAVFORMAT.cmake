
########################################
# Find AV format
pkg_check_modules(AVFORMAT libavformat)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  AVFORMAT
  REQUIRED_VARS AVFORMAT_FOUND)
