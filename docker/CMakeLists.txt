set(ubuntu_package_filename ${PROJECT_NAME}-${SkyPlanner_VERSION}-ubuntu15.04.deb)
set(ubuntu_package_file ${CMAKE_CURRENT_BINARY_DIR}/${ubuntu_package_filename})
add_subdirectory(Ubuntu-15.04-build/)
add_subdirectory(Ubuntu-15.04-run/)
