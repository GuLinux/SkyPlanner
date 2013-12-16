#define BOOST_TEST_MAIN
#include "test_helper.h"
#include <ephemeris.h>
#include <boost/date_time.hpp>

using namespace std;



BOOST_AUTO_TEST_CASE(TestAndromedaEquatorialToAltAz)
{
  double andromedaArAsRad = 0.186314;
  double andromedaDecAsRad = 0.720239;
  double latitude = 44.6411166497765;
  double longitude = 9.2164134979248;
  Ephemeris ephemeris({Angle::degrees(latitude), Angle::degrees(longitude)});
  Coordinates::AltAzimuth altAz = ephemeris.arDec2altAz({Angle::radians(andromedaArAsRad), Angle::radians(andromedaDecAsRad)}, boost::posix_time::time_from_string("2013-11-28 20:04:00"));
  cerr << "alt: " << altAz.altitude.degrees() << ", az: " << altAz.azimuth.degrees() << endl;
}
