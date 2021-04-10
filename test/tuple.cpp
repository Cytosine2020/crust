#include "gtest/gtest.h"

#include "utility.hpp"
#include "tuple.hpp"
#include "cmp.hpp"


using namespace crust;
using namespace cmp;


namespace {
class A {
};

class B {
};
}


GTEST_TEST(tuple, size_zero) {
    CRUST_STATIC_ASSERT(IsMonoState<Tuple<>>::result);

    CRUST_STATIC_ASSERT(Derive<Tuple<>, PartialEq>::result);
    CRUST_STATIC_ASSERT(Derive<Tuple<>, Eq>::result);

    CRUST_STATIC_ASSERT(std::is_trivially_copyable<Tuple<>>::value);
    CRUST_STATIC_ASSERT(std::is_literal_type<Tuple<>>::value);

    EXPECT_TRUE(make_tuple() == make_tuple());
    EXPECT_FALSE(make_tuple() != make_tuple());
    EXPECT_FALSE(make_tuple() < make_tuple());
    EXPECT_TRUE(make_tuple() <= make_tuple());
    EXPECT_FALSE(make_tuple() > make_tuple());
    EXPECT_TRUE(make_tuple() >= make_tuple());
    EXPECT_EQ(operator_partial_cmp(make_tuple(), make_tuple()), make_some(make_equal()));
    EXPECT_EQ(operator_cmp(make_tuple(), make_tuple()), make_equal());
}

GTEST_TEST(tuple, size_one) {
    CRUST_STATIC_ASSERT(!IsMonoState<Tuple<A>>::result);

    CRUST_STATIC_ASSERT(!Derive<Tuple<A>, PartialEq>::result);
    CRUST_STATIC_ASSERT(!Derive<Tuple<A>, Eq>::result);

    CRUST_STATIC_ASSERT(std::is_trivially_copyable<Tuple<A>>::value);
    CRUST_STATIC_ASSERT(std::is_literal_type<Tuple<A>>::value);

    CRUST_STATIC_ASSERT(Derive<Tuple<i32>, PartialEq>::result);
    CRUST_STATIC_ASSERT(Derive<Tuple<i32>, Eq>::result);

    EXPECT_TRUE(make_tuple(0) == make_tuple(0));
    EXPECT_FALSE(make_tuple(0) != make_tuple(0));
    EXPECT_FALSE(make_tuple(0) < make_tuple(0));
    EXPECT_TRUE(make_tuple(0) <= make_tuple(0));
    EXPECT_FALSE(make_tuple(0) > make_tuple(0));
    EXPECT_TRUE(make_tuple(0) >= make_tuple(0));
    EXPECT_EQ(operator_partial_cmp(make_tuple(0), make_tuple(0)), make_some(make_equal()));
    EXPECT_EQ(operator_cmp(make_tuple(0), make_tuple(0)), make_equal());

    auto tuple = make_tuple(0);

    EXPECT_EQ(tuple.get<0>(), 0);

    ++tuple.get<0>();

    EXPECT_EQ(tuple.get<0>(), 1);
}

GTEST_TEST(tuple, size_two) {
    CRUST_STATIC_ASSERT(!IsMonoState<Tuple<A, B>>::result);

    CRUST_STATIC_ASSERT(!Derive<Tuple<A, B>, PartialEq>::result);
    CRUST_STATIC_ASSERT(!Derive<Tuple<A, B>, Eq>::result);

    CRUST_STATIC_ASSERT(std::is_trivially_copyable<Tuple<A, B>>::value);
    CRUST_STATIC_ASSERT(std::is_literal_type<Tuple<A, B>>::value);

    CRUST_STATIC_ASSERT(Derive<Tuple<i32, char>, PartialEq>::result);
    CRUST_STATIC_ASSERT(Derive<Tuple<i32, char>, Eq>::result);

    auto tuple = make_tuple(0, 'a');

    EXPECT_TRUE(tuple.get<0>() == 0);
    EXPECT_TRUE(tuple.get<1>() == 'a');

    ++tuple.get<0>();
    ++tuple.get<1>();

    EXPECT_TRUE(std::get<0>(tuple) == 1);
    EXPECT_TRUE(std::get<1>(tuple) == 'b');

    i32 a;
    char b;

    let(a, b) = move(tuple);

    EXPECT_EQ(a, 1);
    EXPECT_EQ(b, 'b');

    EXPECT_TRUE(make_tuple(true) != make_tuple(false));
    EXPECT_TRUE(make_tuple(1) > make_tuple(0));
    EXPECT_TRUE(make_tuple(0, 'b') > make_tuple(0, 'a'));

    EXPECT_TRUE(make_tuple(0, 1).cmp(make_tuple(0, 0)) == make_greater());
}
