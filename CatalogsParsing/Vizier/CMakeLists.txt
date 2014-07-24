INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/models)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE(${CMAKE_SOURCE_DIR}/src/models/source_files.cmake)
set(vizier_srcs vizier.cpp ${SkyPlanner_MODELS_SRCS}
    ${CMAKE_SOURCE_DIR}/src/types.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/utils.cpp
    ${CMAKE_SOURCE_DIR}/src/constellationfinder.cpp
    ${CMAKE_SOURCE_DIR}/src/ephemeris.cpp
    ${CMAKE_SOURCE_DIR}/src/session.cpp
)
add_library(vizier STATIC ${vizier_srcs})
set(vizier_libraries vizier ${Boost_LIBRARIES} ${Wt_LIBRARIES} nova)

add_subdirectory(Barnard)
