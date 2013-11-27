#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include <astro/degrees.h>

BOOST_AUTO_TEST_CASE(TestDegreesConstructions)
{
  Degrees d1(123.443);
  Degrees d2(123.443);
  BOOST_REQUIRE_CLOSE(123.443, static_cast<double>(d1), 0.0001);
}