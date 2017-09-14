find_package(PkgConfig)

########################################
# Find libswscale format
pkg_check_modules(SWSCALE libswscale)

find_package_handle_standard_args(
  SWSCALE
  REQUIRED_VARS SWSCALE_FOUND)
