configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Dockerfile Dockerfile @ONLY)
add_custom_target(docker_postgres_skyplanner_img docker build -t skyplanner-postgres-img ${CMAKE_CURRENT_SOURCE_DIR})
