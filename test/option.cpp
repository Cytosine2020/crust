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

    i32 x;
    if ((let_enum<Some<i32>>(x) = make_some(1))) {
        EXPECT_EQ(x, 1);
    } else {
        GTEST_FAIL();
    }

    if ((let_enum<Some<i32>>(x) = make_none<i32>())) {
        GTEST_FAIL();
    }
}
