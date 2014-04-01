#define BOOST_TEST_MAIN
#include "test_helper.h"
#include "types.h"

using namespace std;

BOOST_AUTO_TEST_CASE(TestNegativeAngles)
{
  BOOST_REQUIRE_EQUAL( "90°  0' 0.0\"", Angle::degrees(90).printable());
  BOOST_REQUIRE_EQUAL( "-90°  0' 0.0\"", Angle::degrees(-90).printable());
  BOOST_REQUIRE_EQUAL( "-0° 12' 0.0\"", Angle::degrees(-0.2).printable());
  BOOST_REQUIRE_EQUAL( "0° 12' 0.0\"", Angle::degrees(0.2).printable());

  BOOST_REQUIRE_EQUAL( "6h  0m 0.0s", Angle::degrees(90).printable(Angle::Hourly));
  BOOST_REQUIRE_EQUAL( "-6h  0m 0.0s", Angle::degrees(-90).printable(Angle::Hourly));
  BOOST_REQUIRE_EQUAL( "-0h  0m 48.0s", Angle::degrees(-0.2).printable(Angle::Hourly));
  BOOST_REQUIRE_EQUAL( "0h  0m 48.0s", Angle::degrees(0.2).printable(Angle::Hourly));

}
