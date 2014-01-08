include(   ${QT_USE_FILE}   )
add_executable(sac_asterisms_importer asterisms.cpp)
target_link_libraries(sac_asterisms_importer ${QT_LIBRARIES} ${QT_QTSQL_LIBRARY})
