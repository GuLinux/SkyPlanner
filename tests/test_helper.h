#ifndef BOOST_TESTS_HELPER
#define BOOST_TESTS_HELPER
#include <boost/test/unit_test.hpp>
#ifdef IN_KDEVELOP_PARSER
#define BOOST_CHECK_WITH_ARGS_IMPL(...) (void) __VA_ARGS__
#endif
#endif
