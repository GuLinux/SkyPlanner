add_custom_target(docker_image_ubuntu docker build -t skyplanner_ubuntu_builder_build Ubuntu-15.04-build WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_custom_target(docker_create_container_ubuntu COMMAND docker create -v ${CMAKE_SOURCE_DIR}:/SkyPlanner:ro -v ${CMAKE_CURRENT_BINARY_DIR}:/SkyPlanner-dist --name skyplanner_ubuntu_builder skyplanner_ubuntu_builder_build DEPENDS docker_image_ubuntu)
add_custom_target(docker_build_ubuntu COMMAND docker start -a -i skyplanner_ubuntu_builder)
add_custom_target(docker_clean_ubuntu docker rm skyplanner_ubuntu_builder )