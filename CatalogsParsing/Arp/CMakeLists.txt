include(   ${QT_USE_FILE}   )
add_executable(arp_importer arp.cpp)
target_link_libraries(arp_importer ${QT_LIBRARIES} ${QT_QTSQL_LIBRARY})
