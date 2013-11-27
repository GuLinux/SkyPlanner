include_directories(models)
set(NgcIcCatalog_SRCS
  forecast.cpp
  placewidget.cpp
  astrosessiontab.cpp
  astrosessionslisttab.cpp
  astrosessionspage.cpp
  astroplanner.cpp
  ngcresource.cpp
  session.cpp
  main.cpp
)

set(AstroCpp_SRCS
  AstroCpp/RiseSet.cpp
  AstroCpp/AstroOps.cpp
  AstroCpp/PlanetData.cpp
  AstroCpp/Pluto.cpp
  AstroCpp/MathOps.cpp
  AstroCpp/Lunar.cpp
  AstroCpp/Vsop.cpp
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

set(Gum_Astro_SRCS 
  astro/degrees.cpp
  astro/radian.cpp
  astro/celestialtoaltaz.cpp
  astro/rightascension.cpp
)
add_executable(NgcIcCatalog ${NgcIcCatalog_SRCS} ${NgcIcCatalog_MODELS_SRCS} ${AstroCpp_SRCS} ${Gum_Astro_SRCS})
target_link_libraries(NgcIcCatalog ${Wt_LIBRARIES} ${Boost_LIBRARIES} )
install(TARGETS NgcIcCatalog RUNTIME DESTINATION bin)

