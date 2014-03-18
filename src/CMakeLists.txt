include_directories(models)

include(aaplus/source_files.cmake)
include(widgets/source_files.cmake)
set(LIBHARU_PREFIX /usr CACHE PATH "LibHaru path prefix")
include_directories(${LIBHARU_PREFIX}/include/)
set(SkyPlanner_SRCS
  skyplanner.cpp
  forecast.cpp
  placewidget.cpp
  astrosessiontab.cpp
  astrosessionslisttab.cpp
  astrosessionspage.cpp
  ngcresource.cpp
  session.cpp
  ephemeris.cpp
  types.cpp
  selectobjectswidget.cpp
  main.cpp
  utils/utils.cpp
  utils/autopostresource.cpp
  utils/curl.cpp
  telescopespage.cpp
  constellationfinder.cpp
  exportastrosessionresource.cpp
  usersettingspage.cpp
  sendfeedbackpage.cpp
  Wt-Commons/whtmltemplateslocalizedstrings.cpp
  Wt-Commons/wform.cpp
  homepage.cpp
)

set(SkyPlanner_MODELS_SRCS
  models/catalogue.cpp
  models/astrosessionobject.cpp
  models/astrosession.cpp
  models/telescope.cpp
  models/user.cpp
  models/nebuladenomination.cpp
  models/ngcobject.cpp
  models/usersetting.cpp
)




add_executable(SkyPlanner ${SkyPlanner_SRCS} ${SkyPlanner_MODELS_SRCS} ${AAPLUS_SRCS} ${Widgets_SRCS} )
target_link_libraries(SkyPlanner ${Wt_LIBRARIES} ${Boost_LIBRARIES} ${HARU_LIBRARY} ${WIN32_LIBS} curl )
install(TARGETS SkyPlanner RUNTIME DESTINATION bin)

