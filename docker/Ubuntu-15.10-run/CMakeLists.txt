configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Dockerfile Dockerfile @ONLY)
add_custom_target(docker_ubuntu_run_img docker build -t skyplanner-ubuntu-run-img-${DOCKER_SUFFIX} -f ${CMAKE_CURRENT_BINARY_DIR}/Dockerfile ${CMAKE_CURRENT_BINARY_DIR}/.. DEPENDS docker_build_ubuntu)
