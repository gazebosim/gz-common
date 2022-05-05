add_subdirectory(src)
# add_subdirectory(include)

install(
  DIRECTORY include/ignition
  DESTINATION ${IGN_INCLUDE_INSTALL_DIR_FULL})

install(
  DIRECTORY include/gz
  DESTINATION ${IGN_INCLUDE_INSTALL_DIR_FULL})
