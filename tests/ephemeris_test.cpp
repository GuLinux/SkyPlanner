#define BOOST_TEST_MAIN
#include "test_helper.h"
#include <ephemeris.h>
#include <boost/date_time.hpp>

using namespace std;



BOOST_AUTO_TEST_CASE(TestSunRiseSet)
{
  double latitude = 44.6427;
  double longitude = 9.2150;
  boost::posix_time::ptime when({2014, 03, 29});
  Ephemeris ephemeris({Angle::degrees(latitude), Angle::degrees(longitude)}, {});
  Ephemeris::RiseTransitSet sun = ephemeris.sun(when);
  Ephemeris::RiseTransitSet astroTwilight = ephemeris.astronomicalTwilight(when);
  Ephemeris::RiseTransitSet moon = ephemeris.moon(when);
  auto darkness = ephemeris.darknessHours(when );
  Ephemeris::LunarPhase phase = ephemeris.moonPhase(when);
  double firstFraction = phase.illuminated_fraction;
  for(int i=0; i<1000; i++) {
    phase = ephemeris.moonPhase(when);
    BOOST_REQUIRE_CLOSE(firstFraction, phase.illuminated_fraction, 0.0001);
  }

  cerr << "Sun: rise=" << sun.rise << ", transit=" << sun.transit << ", set=" << sun.set << endl;
  cerr << "moon: rise=" << moon.rise << ", transit=" << moon.transit << ", set=" << moon.set << endl;
  cerr << "Astronomical twilight: rise=" << astroTwilight.rise << ", transit=" << astroTwilight.transit << ", set=" << astroTwilight.set << endl;
  cerr << "Darkness: begin=" << darkness.begin << ", end=" << darkness.end << ", duration=" << darkness.duration << endl;
}
