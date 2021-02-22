#include "gtest/gtest.h"

#include "utility.hpp"
#include "tuple.hpp"
#include "cmp.hpp"


using namespace crust;

class A {
};

class B {
};

GTEST_TEST(tuple, tuple) {
    CRUST_STATIC_ASSERT(!CRUST_DERIVE(CRUST_ECHO(Tuple<A, B>), PartialEq));
    CRUST_STATIC_ASSERT(!CRUST_DERIVE(CRUST_ECHO(Tuple<A, B>), Eq));

    CRUST_STATIC_ASSERT(std::is_literal_type<Tuple<>>::value);

    auto tuple = make_tuple(0, 'a');

    EXPECT_TRUE(tuple.get<0>() == 0);
    EXPECT_TRUE(tuple.get<1>() == 'a');

    ++tuple.get<0>();
    ++tuple.get<1>();

    EXPECT_TRUE(std::get<0>(tuple) == 1);
    EXPECT_TRUE(std::get<1>(tuple) == 'b');

    EXPECT_TRUE(make_tuple() == make_tuple());

    EXPECT_TRUE(make_tuple(true) != make_tuple(false));
    EXPECT_TRUE(make_tuple(1) > make_tuple(0));
    EXPECT_TRUE(make_tuple(0, 'b') > make_tuple(0, 'a'));

    EXPECT_TRUE(make_tuple(0, 1).cmp(make_tuple(0, 0)) == Ordering::greater());
}
