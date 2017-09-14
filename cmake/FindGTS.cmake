
########################################
# Find GNU Triangulation Surface Library
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTS gts)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  GTS
  REQUIRED_VARS GTS_FOUND)
