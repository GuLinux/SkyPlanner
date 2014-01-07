include_directories(models)

include(aaplus/source_files.cmake)
include(widgets/source_files.cmake)
set(LIBHARU_PREFIX /usr CACHE PATH "LibHaru path prefix")
include_directories(${LIBHARU_PREFIX}/include/)
set(SkyPlanner_SRCS
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
  utils/autopostresource.cpp
  telescopespage.cpp
  constellationfinder.cpp
  printableastrosessionresource.cpp
)

set(SkyPlanner_MODELS_SRCS
  models/astrosessionobject.cpp
  models/astrosession.cpp
  models/telescope.cpp
  models/user.cpp
  models/nebuladenomination.cpp
  models/ngcobject.cpp
)


add_executable(SkyPlanner ${SkyPlanner_SRCS} ${SkyPlanner_MODELS_SRCS} ${AAPLUS_SRCS} ${Widgets_SRCS} )
target_link_libraries(SkyPlanner ${Wt_LIBRARIES} ${Boost_LIBRARIES} ${LIBHARU_PREFIX}/lib/libhpdf.so )
install(TARGETS SkyPlanner RUNTIME DESTINATION bin)

