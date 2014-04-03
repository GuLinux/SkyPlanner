include(   ${QT_USE_FILE}   )
add_definitions(-DTESTS_NO_WT)
add_executable(shakhbazian_importer shakhbazian.cpp ${CMAKE_SOURCE_DIR}/src/types.cpp)
target_link_libraries(shakhbazian_importer ${QT_LIBRARIES} ${QT_QTSQL_LIBRARY} nova ${Boost_LIBRARIES})
