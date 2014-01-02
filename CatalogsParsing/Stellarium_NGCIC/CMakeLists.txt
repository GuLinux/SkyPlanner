find_package(Qt4 REQUIRED)
include(   ${QT_USE_FILE}   )
add_executable(stellarium_importer stellarium_ngcic.cpp)
target_link_libraries(stellarium_importer ${QT_LIBRARIES} ${QT_QTSQL_LIBRARY})
