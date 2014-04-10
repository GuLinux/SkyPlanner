include(   ${QT_USE_FILE}   )
add_definitions(-DTESTS_NO_WT)
include_directories(${CMAKE_SOURCE_DIR}/src/models)
include(${CMAKE_SOURCE_DIR}/src/models/source_files.cmake)
add_executable(shakhbazian_importer shakhbazian.cpp
  ${CMAKE_SOURCE_DIR}/src/types.cpp
  ${SkyPlanner_MODELS_SRCS}
  ${CMAKE_SOURCE_DIR}/src/ephemeris.cpp
  ${CMAKE_SOURCE_DIR}/src/utils/utils.cpp
  ${CMAKE_SOURCE_DIR}/src/constellationfinder.cpp
)
target_link_libraries(shakhbazian_importer ${QT_LIBRARIES} ${QT_QTSQL_LIBRARY} nova ${Boost_LIBRARIES} ${Wt_LIBRARIES})
