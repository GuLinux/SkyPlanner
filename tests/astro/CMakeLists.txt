register_unit_test(coordinates_conversion_test
  coordinates_conversion_test.cpp
  ${CMAKE_SOURCE_DIR}/src/astro/degrees.cpp
  ${CMAKE_SOURCE_DIR}/src/astro/radian.cpp
)
link_unit_test(coordinates_conversion_test)