include(   ${QT_USE_FILE}   )
add_executable(mcg_importer mcg.cpp)
target_link_libraries(mcg_importer ${QT_LIBRARIES} ${QT_QTSQL_LIBRARY})
