#include "gtest/gtest.h"

#include "crust/cmp.hpp"
#include "crust/tuple.hpp"
#include "crust/utility.hpp"


using namespace crust;
using namespace cmp;


namespace {
class A {};

class B {};
} // namespace


GTEST_TEST(tuple, size_zero) {
  crust_static_assert(IsZeroSizedTypeVal<Tuple<>>::result);
  crust_static_assert(IsZeroSizedTypeVal<Tuple<Tuple<>>>::result);
  crust_static_assert(IsZeroSizedTypeVal<Tuple<Tuple<>, Tuple<>>>::result);
  crust_static_assert(
      IsZeroSizedTypeVal<Tuple<Tuple<>, Tuple<>, Tuple<>>>::result);

  crust_static_assert(sizeof(Tuple<>) == 1);
  crust_static_assert(sizeof(Tuple<Tuple<>>) == 1);
  crust_static_assert(sizeof(Tuple<Tuple<>, Tuple<>>) == 1);
  crust_static_assert(sizeof(Tuple<Tuple<>, Tuple<>, Tuple<>>) == 1);
  crust_static_assert(sizeof(Tuple<i32, Tuple<>, Tuple<>>) == sizeof(i32));
  crust_static_assert(sizeof(Tuple<Tuple<>, i32, Tuple<>>) == sizeof(i32));
  crust_static_assert(sizeof(Tuple<Tuple<>, Tuple<>, i32>) == sizeof(i32));

  crust_static_assert(Derive<Tuple<>, PartialEq>::result);
  crust_static_assert(Derive<Tuple<>, Eq>::result);

  crust_static_assert(std::is_trivially_copyable<Tuple<>>::value);
  crust_static_assert(std::is_literal_type<Tuple<>>::value);

  crust_static_assert(make_tuple() == make_tuple());
  crust_static_assert(!(make_tuple() != make_tuple()));
  crust_static_assert(!(make_tuple() < make_tuple()));
  crust_static_assert(make_tuple() <= make_tuple());
  crust_static_assert(!(make_tuple() > make_tuple()));
  crust_static_assert(make_tuple() >= make_tuple());
  EXPECT_EQ(
      operator_partial_cmp(make_tuple(), make_tuple()),
      make_some(make_equal()));
  EXPECT_EQ(operator_cmp(make_tuple(), make_tuple()), make_equal());
}

GTEST_TEST(tuple, size_one) {
  crust_static_assert(!IsZeroSizedTypeVal<Tuple<A>>::result);

  crust_static_assert(!Derive<Tuple<A>, PartialEq>::result);
  crust_static_assert(!Derive<Tuple<A>, Eq>::result);

  crust_static_assert(std::is_trivially_copyable<Tuple<A>>::value);
  crust_static_assert(std::is_literal_type<Tuple<A>>::value);

  crust_static_assert(Derive<Tuple<i32>, PartialEq>::result);
  crust_static_assert(Derive<Tuple<i32>, Eq>::result);

  crust_static_assert(make_tuple(0) == make_tuple(0));
  crust_static_assert(!(make_tuple(0) != make_tuple(0)));
  crust_static_assert(!(make_tuple(0) < make_tuple(0)));
  crust_static_assert(make_tuple(0) <= make_tuple(0));
  crust_static_assert(!(make_tuple(0) > make_tuple(0)));
  crust_static_assert(make_tuple(0) >= make_tuple(0));
  EXPECT_EQ(
      operator_partial_cmp(make_tuple(0), make_tuple(0)),
      make_some(make_equal()));
  EXPECT_EQ(operator_cmp(make_tuple(0), make_tuple(0)), make_equal());

  auto tuple = make_tuple(0);

  EXPECT_EQ(tuple.get<0>(), 0);

  ++tuple.get<0>();

  EXPECT_EQ(tuple.get<0>(), 1);
}

GTEST_TEST(tuple, size_two) {
  crust_static_assert(!IsZeroSizedTypeVal<Tuple<A, B>>::result);

  crust_static_assert(!Derive<Tuple<A, B>, PartialEq>::result);
  crust_static_assert(!Derive<Tuple<A, B>, Eq>::result);

  crust_static_assert(std::is_trivially_copyable<Tuple<A, B>>::value);
  crust_static_assert(std::is_literal_type<Tuple<A, B>>::value);

  crust_static_assert(Derive<Tuple<i32, char>, PartialEq>::result);
  crust_static_assert(Derive<Tuple<i32, char>, Eq>::result);

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

  crust_static_assert(make_tuple(true) != make_tuple(false));
  crust_static_assert(make_tuple(1) > make_tuple(0));
  crust_static_assert(make_tuple(0, 'b') > make_tuple(0, 'a'));

  EXPECT_TRUE(make_tuple(0, 1).cmp(make_tuple(0, 0)) == make_greater());
}
