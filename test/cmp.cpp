#include "gtest/gtest.h"

#include "utility.hpp"
#include "cmp.hpp"


using namespace crust;
using namespace cmp;


GTEST_TEST(cmp, derive) {
    CRUST_STATIC_ASSERT(CRUST_DERIVE(bool, PartialEq));
    CRUST_STATIC_ASSERT(CRUST_DERIVE(u8, PartialEq));
    CRUST_STATIC_ASSERT(CRUST_DERIVE(i8, PartialEq));
    CRUST_STATIC_ASSERT(CRUST_DERIVE(u16, PartialEq));
    CRUST_STATIC_ASSERT(CRUST_DERIVE(i16, PartialEq));
    CRUST_STATIC_ASSERT(CRUST_DERIVE(u32, PartialEq));
    CRUST_STATIC_ASSERT(CRUST_DERIVE(i32, PartialEq));
    CRUST_STATIC_ASSERT(CRUST_DERIVE(u64, PartialEq));
    CRUST_STATIC_ASSERT(CRUST_DERIVE(i64, PartialEq));
    CRUST_STATIC_ASSERT(CRUST_DERIVE(usize, PartialEq));
    CRUST_STATIC_ASSERT(CRUST_DERIVE(isize, PartialEq));

    CRUST_STATIC_ASSERT(CRUST_DERIVE(bool, Eq));
    CRUST_STATIC_ASSERT(CRUST_DERIVE(u8, Eq));
    CRUST_STATIC_ASSERT(CRUST_DERIVE(i8, Eq));
    CRUST_STATIC_ASSERT(CRUST_DERIVE(u16, Eq));
    CRUST_STATIC_ASSERT(CRUST_DERIVE(i16, Eq));
    CRUST_STATIC_ASSERT(CRUST_DERIVE(u32, Eq));
    CRUST_STATIC_ASSERT(CRUST_DERIVE(i32, Eq));
    CRUST_STATIC_ASSERT(CRUST_DERIVE(u64, Eq));
    CRUST_STATIC_ASSERT(CRUST_DERIVE(i64, Eq));
    CRUST_STATIC_ASSERT(CRUST_DERIVE(usize, Eq));
    CRUST_STATIC_ASSERT(CRUST_DERIVE(isize, Eq));
}


GTEST_TEST(cmp, ordering) {
    EXPECT_TRUE(make_less() == make_less());
    EXPECT_TRUE(make_equal() == make_equal());
    EXPECT_TRUE(make_greater() == make_greater());
    EXPECT_TRUE(make_less() < make_equal());
    EXPECT_TRUE(make_less() < make_greater());
    EXPECT_TRUE(make_equal() < make_greater());

    EXPECT_EQ(make_equal().then(make_less()), make_less());
    EXPECT_EQ(make_less().then(make_equal()), make_less());
    EXPECT_EQ(make_greater().then(make_equal()), make_greater());
    EXPECT_EQ(make_equal().then_with(make_fn([]() {
        return make_less();
    })), make_less());
    EXPECT_EQ(make_less().then_with(make_fn([]() {
        return make_equal();
    })), make_less());
    EXPECT_EQ(make_greater().then_with(make_fn([]() {
        return make_equal();
    })), make_greater());
}


GTEST_TEST(cmp, min_max) {
    EXPECT_EQ(make_tuple(0, 'b'), min(make_tuple(0, 'b'), make_tuple(1, 'a')));
    EXPECT_EQ(make_tuple(1, 'a'), max(make_tuple(0, 'b'), make_tuple(1, 'a')));

    EXPECT_EQ(make_tuple(0, 'b'), min_by_key(
            make_tuple(0, 'b'), make_tuple(1, 'a'), make_fn([](const Tuple<i32, char> &value) {
                return value.get<0>();
            }))
    );

    EXPECT_EQ(make_tuple(1, 'a'), min_by_key(
            make_tuple(0, 'b'), make_tuple(1, 'a'), make_fn([](const Tuple<i32, char> &value) {
                return value.get<1>();
            }))
    );

    EXPECT_EQ(make_tuple(1, 'a'), max_by_key(
            make_tuple(0, 'b'), make_tuple(1, 'a'), make_fn([](const Tuple<i32, char> &value) {
                return value.get<0>();
            }))
    );

    EXPECT_EQ(make_tuple(0, 'b'), max_by_key(
            make_tuple(0, 'b'), make_tuple(1, 'a'), make_fn([](const Tuple<i32, char> &value) {
                return value.get<1>();
            }))
    );
}
