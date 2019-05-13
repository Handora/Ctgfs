/* author: weifeng */

#include "util/util.h"
#include <gtest/gtest.h>

TEST(UtilTest, CTGWARN) {
  int x = 2;
  CTG_WARN("2 * 2 = %d", x * x);
  CTG_DEBUG("2 * 2 = %d", x * x);
  CTG_INFO("2 * 2 = %d", x * x);
  CTG_ERROR("2 * 2 = %d", x * x);
}