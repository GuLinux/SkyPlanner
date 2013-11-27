#define BOOST_TEST_MAIN
#include "test_helper.h"
#include <astro/degrees.h>

BOOST_AUTO_TEST_CASE(TestDegreesConstructionsAndConversion)
{
  Degrees d1(123.443);
  Degrees d2({123, 7, 23.0});
  BOOST_REQUIRE_CLOSE(123.443, static_cast<double>(d1), 0.0001);
  BOOST_REQUIRE_CLOSE(123.123, static_cast<double>(d2), 0.0001);
  Degrees::Sexagesimal sexagesimal = d2;
  BOOST_REQUIRE_EQUAL(123, sexagesimal.degrees);
  BOOST_REQUIRE_EQUAL(7, sexagesimal.minutes);
  BOOST_REQUIRE_CLOSE(23., sexagesimal.seconds, 0.0001);
  Radian d1AsRad = d1;
  BOOST_REQUIRE_CLOSE(static_cast<double>(Radian{2.15448679}), static_cast<double>(d1AsRad), 0.0001);
}