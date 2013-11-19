set(NgcIcCatalog_SRCS nebuladenomination.cpp ngcobject.cpp ngcresource.cpp session.cpp main.cpp)
add_executable(NgcIcCatalog ${NgcIcCatalog_SRCS})
target_link_libraries(NgcIcCatalog ${Wt_LIBRARIES} ${Boost_LIBRARIES} )
install(TARGETS NgcIcCatalog RUNTIME DESTINATION bin)

