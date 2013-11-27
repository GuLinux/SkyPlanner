#define BOOST_TEST_MAIN
#include "test_helper.h"
#include <astro/degrees.h>
#include <astro/radian.h>
#include <astro/rightascension.h>
#include <astro/celestialtoaltaz.h>
using namespace std;

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



BOOST_AUTO_TEST_CASE(TestSideralTime)
{
  CelestialToAltAz converter({Degrees{{44, 22}}, Degrees{{15}}});
  BOOST_CHECK_CLOSE_FRACTION(static_cast<double>(RightAscension{{16,38,23}}), converter.localSideralTime(boost::posix_time::time_from_string("2013-11-27 11:12:16")), 0.1);
  BOOST_CHECK_CLOSE_FRACTION(static_cast<double>(RightAscension{{7,55,12}}), converter.localSideralTime(boost::posix_time::time_from_string("2013-06-01 15:18:00")), 0.1);
  BOOST_CHECK_CLOSE_FRACTION(static_cast<double>(RightAscension{{22,58,0}}), converter.localSideralTime(boost::posix_time::time_from_string("2013-03-21 11:02:00")), 0.1);
}

BOOST_AUTO_TEST_CASE(TestCelestialToAltAzConversion)
{
  CelestialToAltAz converter({Degrees{{44, 22}}, Degrees{{12,34}}});
  CelestialToAltAz::RADecCoordinates object{RightAscension{{12,33}}, Degrees{{34,33}}};
  CelestialToAltAz::AltAzCoordinates result = converter.coordinatesAt(object, boost::posix_time::time_from_string("2013-11-22 11:31:33"));
  Degrees altitude = result.alt;
  Degrees azimuth = result.azimuth;
  BOOST_REQUIRE_CLOSE(55.25341545113157, static_cast<double>(altitude), 0.0001);
  BOOST_REQUIRE_CLOSE(268.95467191935677, static_cast<double>(azimuth), 0.0001);
}

BOOST_AUTO_TEST_CASE(InteractiveTestCelestialToAltAzConversion)
{
  string when;
  Degrees latitude{45.64666667};
  Degrees longitude{9.19};
  RightAscension::RA ra;
  Degrees::Sexagesimal ss;
  cout << "Enter Right ascension: ";
  cin >> ra.hours >> ra.minutes >> ra.seconds;
  cout << "Enter declination: ";
  cin >> ss.degrees >> ss.minutes >> ss.seconds;
  cout << "Enter a date: " << endl;
//  getline(cin, when);
  when = "2013-11-27 20:25:16";
  CelestialToAltAz converter( {Degrees{latitude}, Degrees{longitude}});
  CelestialToAltAz::RADecCoordinates object {RightAscension{ra}, Degrees{ss}};
  CelestialToAltAz::AltAzCoordinates result = converter.coordinatesAt(object, boost::posix_time::time_from_string(when));
  Degrees altitude = result.alt;
  Degrees azimuth = result.azimuth;
  cout << "Altitude: " << static_cast<double>(altitude) << ", azimuth: " << static_cast<double>(azimuth);
}
