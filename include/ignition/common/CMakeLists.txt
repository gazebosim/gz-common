# Avoid to install the win_dirent header when not in windows
if(WIN32)
  ign_install_all_headers()
else()
  ign_install_all_headers(EXCLUDE_FILES "win_dirent.h")
endif()
