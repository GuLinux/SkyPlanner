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
  selectobjectswidget.cpp
  main.cpp
  utils/autopostresource.cpp
  utils/curl.cpp
  telescopespage.cpp
  exportastrosessionresource.cpp
  usersettingspage.cpp
  sendfeedbackpage.cpp
  homepage.cpp
  dss.cpp
  geocoder.cpp
  astrosessionpreview.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/style.css.cpp
)
include(models/source_files.cmake)

set(SkyPlanner_CORE_SRCS
  ${SkyPlanner_MODELS_SRCS}
    types.cpp
    utils/utils.cpp
    constellationfinder.cpp
    ephemeris.cpp
    session.cpp
    urls.cpp
)
file(GLOB openweather_SRCS openweather/*cpp)
add_custom_command(OUTPUT style.css.cpp COMMAND ${CMAKE_SOURCE_DIR}/scripts/create-style-css
                                              ${CMAKE_SOURCE_DIR}/src/style.css.cpp.in 
                                              ${CMAKE_SOURCE_DIR}/resources/style.css 
                                              ${CMAKE_CURRENT_BINARY_DIR}/style.css.cpp
                                              DEPENDS ${CMAKE_SOURCE_DIR}/src/style.css.cpp.in ${CMAKE_SOURCE_DIR}/resources/style.css 
                                              )
add_library(SkyPlanner_CORE STATIC ${SkyPlanner_CORE_SRCS})
add_subdirectory(webservice)
add_executable(SkyPlanner ${SkyPlanner_SRCS} ${Wt_Commons_Library_SRCS} ${openweather_SRCS} ${Widgets_SRCS} )
target_link_libraries(SkyPlanner SkyPlanner_CORE SkyPlanner_WebService Wt_Commons_Library ${Wt_LIBRARIES} ${Boost_LIBRARIES} ${HARU_LIBRARY} curl ${GRAPHICSMAGICK_LDFLAGS} nova ${EXTRA_LIBS})
install(TARGETS SkyPlanner RUNTIME DESTINATION bin)

