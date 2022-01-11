#include "gtest/gtest.h"

#include "utility.hpp"
#include "cmp.hpp"
#include "option.hpp"


using namespace crust;


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

    i32 x;
    if ((let<Some<i32>>(x) = make_some(1))) {
        EXPECT_EQ(x, 1);
    } else {
        GTEST_FAIL();
    }

    if ((let<Some<i32>>(x) = make_none<i32>())) {
        GTEST_FAIL();
    }

    // EXPECT_TRUE(*make_some(1234).as_ptr().unwrap_or(0) == 1234);
    EXPECT_TRUE(*make_some(1234).map(make_fn(
        [](const i32 &value) { return &value; }
    )).unwrap_or(0) == 1234);
}
