#include "gtest/gtest.h"
#include <libnova/libnova.h>

ln_date NEW_MOON {2016, 6, 5, 2, 59};
ln_date FIRST_QUARTER {2016, 6, 12, 8, 9};
ln_date FULL_MOON {2016, 6, 20, 11, 2};
ln_date LAST_QUARTER {2016, 6, 27, 18, 18};

class NovaTest : public testing::Test {
public:
  double new_moon, three_days, first_quarter, three_past_first_quarter, full_moon, three_past_full_moon, last_quarter, three_past_last_quarter;
  NovaTest();
  static double phase(ln_date &date) { return ln_get_julian_day(&date); }
};

NovaTest::NovaTest()
  : new_moon{phase(NEW_MOON)},
    three_days{new_moon+3}, 
    first_quarter{phase(FIRST_QUARTER)},
    three_past_first_quarter{first_quarter+3},
    full_moon{phase(FULL_MOON)},
    three_past_full_moon{full_moon+3},
    last_quarter{phase(LAST_QUARTER)},
    three_past_last_quarter{last_quarter+3}
{
}


TEST_F(NovaTest, MoonPhase) {
  ASSERT_NEAR(174, ln_get_lunar_phase(new_moon), 1);
  ASSERT_NEAR(140, ln_get_lunar_phase(three_days), 1);
  ASSERT_NEAR(90, ln_get_lunar_phase(first_quarter), 1);
  ASSERT_NEAR(57, ln_get_lunar_phase(three_past_first_quarter), 1);
  ASSERT_NEAR(4, ln_get_lunar_phase(full_moon), 1);
  ASSERT_NEAR(35, ln_get_lunar_phase(three_past_full_moon), 1);
  ASSERT_NEAR(90, ln_get_lunar_phase(last_quarter), 1);
  ASSERT_NEAR(130, ln_get_lunar_phase(three_past_last_quarter), 1);
}

TEST_F(NovaTest, BrightLimb) {
  ASSERT_LT(ln_get_lunar_bright_limb(new_moon-0.5), 180);
  ASSERT_GT(ln_get_lunar_bright_limb(new_moon), 180);
  ASSERT_GT(ln_get_lunar_bright_limb(three_days), 180);
  ASSERT_GT(ln_get_lunar_bright_limb(first_quarter), 180);
  ASSERT_GT(ln_get_lunar_bright_limb(three_past_first_quarter), 180);
  ASSERT_GT(ln_get_lunar_bright_limb(full_moon-0.5), 180);
  ASSERT_LT(ln_get_lunar_bright_limb(full_moon), 180);
  ASSERT_LT(ln_get_lunar_bright_limb(three_past_full_moon), 180);
  ASSERT_LT(ln_get_lunar_bright_limb(last_quarter), 180);
  ASSERT_LT(ln_get_lunar_bright_limb(three_past_last_quarter), 180);
  ASSERT_LT(ln_get_lunar_bright_limb(three_past_last_quarter+3), 180);
  ASSERT_GT(ln_get_lunar_bright_limb(three_past_last_quarter+4), 180);
}