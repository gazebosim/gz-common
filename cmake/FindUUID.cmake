
#################################################
# Find uuid
find_package(PkgConfig REQUIRED)
pkg_check_modules(UUID uuid)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  UUID
  REQUIRED_VARS UUID_FOUND)
