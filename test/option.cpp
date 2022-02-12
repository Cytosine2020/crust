#include "gtest/gtest.h"

#include "crust/cmp.hpp"
#include "crust/enum.hpp"
#include "crust/option.hpp"
#include "crust/utility.hpp"


using namespace crust;
using ops::bind;
using ops::bind_mut;


GTEST_TEST(option, option) {
  EXPECT_TRUE(make_some(0) == make_some(0));
  EXPECT_FALSE(make_some(0) == make_some(1));
  EXPECT_TRUE(make_some(0) != make_some(1));
  EXPECT_FALSE(make_some(0) != make_some(0));
  EXPECT_TRUE(make_some(0) != None{});
  EXPECT_FALSE(make_some(0) == None{});

  EXPECT_TRUE(make_some(0).is_some());
  EXPECT_FALSE(make_some(0).is_none());
  EXPECT_TRUE(make_none<i32>().is_none());
  EXPECT_FALSE(make_none<i32>().is_some());

  EXPECT_TRUE(*make_some(1234).as_ptr().unwrap_or(0) == 1234);
  EXPECT_TRUE(
      *make_some(1234)
           .map(bind([](const i32 &value) { return &value; }))
           .unwrap_or(0) == 1234);
}
