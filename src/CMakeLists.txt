include_directories(models)

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
  dss.cpp
  geocoder.cpp
)
include(models/source_files.cmake)


add_custom_command(OUTPUT style.css.h COMMAND ${CMAKE_SOURCE_DIR}/scripts/create-style-css
                                              ${CMAKE_SOURCE_DIR}/src/style.css.h.in 
                                              ${CMAKE_SOURCE_DIR}/resources/style.css 
                                              ${CMAKE_SOURCE_DIR}/build/src/style.css.h
                                              )
add_custom_target(clean-style-css COMMAND rm -f ${CMAKE_CURRENT_BINARY_DIR}/style.css.h SOURCES ${CMAKE_SOURCE_DIR}/resources/style.css)
add_custom_target(regen-style-css DEPENDS clean-style-css style.css.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(SkyPlanner ${SkyPlanner_SRCS} ${SkyPlanner_MODELS_SRCS} ${AAPLUS_SRCS} ${Widgets_SRCS} )
target_link_libraries(SkyPlanner ${Wt_LIBRARIES} ${Boost_LIBRARIES} ${HARU_LIBRARY} ${WIN32_LIBS} curl ${GRAPHICSMAGICK_LDFLAGS} nova)
install(TARGETS SkyPlanner RUNTIME DESTINATION bin)

