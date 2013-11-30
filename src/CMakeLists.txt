include_directories(models)

include(aaplus/source_files.cmake)

set(NgcIcCatalog_SRCS
  forecast.cpp
  placewidget.cpp
  astrosessiontab.cpp
  astrosessionslisttab.cpp
  astrosessionspage.cpp
  astroplanner.cpp
  ngcresource.cpp
  session.cpp
  ephemeris.cpp
  types.cpp
  selectobjectswidget.cpp
  main.cpp
  utils/utils.cpp
  WGoogleMap.C
  objectdifficultywidget.cpp
)

set(NgcIcCatalog_MODELS_SRCS
  models/astrosessionobject.cpp
  models/astrosession.cpp
  telescopespage.cpp
  models/telescope.cpp
  models/user.cpp
  models/nebuladenomination.cpp
  models/ngcobject.cpp
)

add_executable(NgcIcCatalog ${NgcIcCatalog_SRCS} ${NgcIcCatalog_MODELS_SRCS} ${AAPLUS_SRCS} )
target_link_libraries(NgcIcCatalog ${Wt_LIBRARIES} ${Boost_LIBRARIES} )
install(TARGETS NgcIcCatalog RUNTIME DESTINATION bin)

