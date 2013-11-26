include_directories(models)
set(NgcIcCatalog_SRCS placewidget.cpp astrosessiontab.cpp astrosessionslisttab.cpp astrosessionspage.cpp astroplanner.cpp ngcresource.cpp session.cpp main.cpp)
set(NgcIcCatalog_MODELS_SRCS models/astrosessionobject.cpp models/astrosession.cpp telescopespage.cpp models/telescope.cpp models/user.cpp models/nebuladenomination.cpp models/ngcobject.cpp)
add_executable(NgcIcCatalog ${NgcIcCatalog_SRCS} ${NgcIcCatalog_MODELS_SRCS} )
target_link_libraries(NgcIcCatalog ${Wt_LIBRARIES} ${Boost_LIBRARIES} )
install(TARGETS NgcIcCatalog RUNTIME DESTINATION bin)

