set(SkyPlanner_WebService_SRCS activesessionsresource.cpp dbo_restresource.cpp ${CMAKE_SOURCE_DIR}/src/Wt-Commons/json_object.cpp)
add_library(SkyPlanner_WebService ${SkyPlanner_WebService_SRCS})

