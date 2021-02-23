#include "gtest/gtest.h"

#include "utility.hpp"
#include "cmp.hpp"


using namespace crust;


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
    auto a = Ordering::equal();

    EXPECT_EQ(a.then_with(make_fn([]() { return Ordering::less(); })), Ordering::less());
}


GTEST_TEST(cmp, max_by_key) {
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
