add_subdirectory(src)
# add_subdirectory(include)

install(
  DIRECTORY include/ignition
  DESTINATION ${GZ_INCLUDE_INSTALL_DIR_FULL})

install(
  DIRECTORY include/gz
  DESTINATION ${GZ_INCLUDE_INSTALL_DIR_FULL})
