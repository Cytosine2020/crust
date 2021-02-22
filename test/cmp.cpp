#include "gtest/gtest.h"

#include "utility.hpp"
#include "cmp.hpp"


using namespace crust;


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
