add_executable(dss-cacher
  dss-cacher.cpp 
  ${CMAKE_SOURCE_DIR}/src/dss.cpp
  ${CMAKE_SOURCE_DIR}/src/widgets/dssimage.cpp 
  ${CMAKE_SOURCE_DIR}/src/utils/curl.cpp
)
target_link_libraries(dss-cacher SkyPlanner_CORE Wt_Commons_Library GuLinux_c++_Commons ${Boost_LIBRARIES} ${Wt_LIBRARIES} curl ${GRAPHICSMAGICK_LDFLAGS} nova)