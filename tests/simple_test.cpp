#define BOOST_TEST_MAIN
#include "test_helper.h"
#include <boost/date_time.hpp>

using namespace std;

BOOST_AUTO_TEST_CASE(TestBoostDate)
{
  boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
  cerr << "year: " << now.date().year() << ", month: " << now.date().month() << ", day: " << now.date().day() << ", day number: " << now.date().day_number() << endl;
  cerr << "hour: " << now.time_of_day().hours() << ", minute: " << now.time_of_day().minutes() << ", second: " << now.time_of_day().seconds() << ", frac: " << now.time_of_day().fractional_seconds() << endl;
}