find_package(Qt4 REQUIRED)
include(   ${QT_USE_FILE}   )
add_executable(abell_importer main.cpp)
target_link_libraries(abell_importer ${QT_LIBRARIES} ${QT_QTSQL_LIBRARY})
