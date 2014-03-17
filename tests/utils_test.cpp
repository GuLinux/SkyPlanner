#define BOOST_TEST_MAIN
#include "test_helper.h"
#include <boost/date_time.hpp>

#include "utils/utils.h"

using namespace std;


BOOST_AUTO_TEST_CASE(TestExponentialPercent)
{
  BOOST_CHECK_EQUAL(1, Utils::exponentialPercentage(-2., 13., 1.3));
  BOOST_CHECK_EQUAL(5, Utils::exponentialPercentage(2., 13., 1.3));
  BOOST_CHECK_EQUAL(9, Utils::exponentialPercentage(4., 13., 1.3));
  BOOST_CHECK_EQUAL(26, Utils::exponentialPercentage(8., 13., 1.3));
  BOOST_CHECK_EQUAL(45, Utils::exponentialPercentage(10., 13., 1.3));
  BOOST_CHECK_EQUAL(76, Utils::exponentialPercentage(12., 13., 1.3));
  BOOST_CHECK_EQUAL(100, Utils::exponentialPercentage(13., 13., 1.3));
  BOOST_CHECK_EQUAL(169, Utils::exponentialPercentage(15., 13., 1.3));
}

BOOST_AUTO_TEST_CASE(TestToCSV)
{
  BOOST_CHECK_EQUAL("foo", Utils::csv("foo"));
  BOOST_CHECK_EQUAL("\"foo\n\"", Utils::csv("foo\n"));
  BOOST_CHECK_EQUAL("\"foo\r\"", Utils::csv("foo\r"));
  BOOST_CHECK_EQUAL("\"foo, bla\"", Utils::csv("foo, bla"));
  BOOST_CHECK_EQUAL("\"foo \"\"bla\"\"\"", Utils::csv("foo \"bla\""));
}
