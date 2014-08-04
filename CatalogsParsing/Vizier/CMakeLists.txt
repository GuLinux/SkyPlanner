INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/models)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE(${CMAKE_SOURCE_DIR}/src/models/source_files.cmake)
set(vizier_srcs vizier.cpp )
add_library(vizier STATIC ${vizier_srcs})
target_link_libraries(vizier SkyPlanner_CORE)
set(vizier_libraries SkyPlanner_CORE vizier ${Boost_LIBRARIES} ${Wt_LIBRARIES} nova)

add_subdirectory(Barnard)
