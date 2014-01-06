include(   ${QT_USE_FILE}   )
add_executable(ugc_importer ugc.cpp)
target_link_libraries(ugc_importer ${QT_LIBRARIES} ${QT_QTSQL_LIBRARY})
