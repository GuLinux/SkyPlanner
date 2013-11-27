#define BOOST_TEST_MAIN
#include "test_helper.h"
#include <astro/degrees.h>
#include <astro/radian.h>
#include <astro/rightascension.h>

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
  Degrees d1BackAsDegrees = d1AsRad;
  BOOST_REQUIRE_CLOSE(123.443, static_cast<double>(d1BackAsDegrees), 0.0001);
}


BOOST_AUTO_TEST_CASE(TestRightAscensionConstructionsAndConversion)
{
  RightAscension ra1{13.33454};
  BOOST_REQUIRE_CLOSE(static_cast<double>(ra1), 13.33454, 0.0001);
  RightAscension ra2{{12, 22, 33.23}};
  BOOST_REQUIRE_CLOSE(static_cast<double>(ra2), 12.37589722, 0.0001);
  RightAscension::RA ra2asRA = ra2;
  BOOST_REQUIRE_EQUAL(12, ra2asRA.hours);
  BOOST_REQUIRE_EQUAL(22, ra2asRA.minutes);
  BOOST_REQUIRE_CLOSE(33.23, ra2asRA.seconds, 0.0001);
  
  Degrees ra2AsDegrees = ra2;
  BOOST_REQUIRE_CLOSE(185.6384, static_cast<double>(ra2AsDegrees), 0.0001);
  Radian ra2AsRad = ra2;
  BOOST_REQUIRE_CLOSE(3.2400013, static_cast<double>(ra2AsRad), 0.0001);
}