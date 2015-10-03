configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Dockerfile Dockerfile @ONLY)
add_custom_command(OUTPUT docker_clean_ubuntu_run_img COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/../docker-cleanup-container skyplanner_ubuntu_run )
add_custom_target(docker_ubuntu_run_img docker build -t skyplanner_ubuntu_run_img -f ${CMAKE_CURRENT_BINARY_DIR}/Dockerfile ${CMAKE_CURRENT_BINARY_DIR}/.. DEPENDS docker_build_ubuntu docker_clean_ubuntu_run_img)
