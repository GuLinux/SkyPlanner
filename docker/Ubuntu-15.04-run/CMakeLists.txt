configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Dockerfile Dockerfile @ONLY)
add_custom_target(docker_clean_ubuntu_run_img docker rm skyplanner_ubuntu_run )
add_custom_target(docker_ubuntu_run_img docker build -t skyplanner_ubuntu_run_img -f ${CMAKE_CURRENT_BINARY_DIR}/Dockerfile ${CMAKE_CURRENT_BINARY_DIR}/.. DEPENDS docker_build_ubuntu)
#add_custom_target(docker_create_daemon_container_ubuntu COMMAND docker create -v ${CMAKE_SOURCE_DIR}:/SkyPlanner:ro -v ${CMAKE_CURRENT_BINARY_DIR}:/SkyPlanner-dist --name skyplanner_ubuntu_run skyplanner_ubuntu_run_img DEPENDS docker_daemon_image_ubuntu)
