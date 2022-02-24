#include "gtest/gtest.h"

#include "crust/clone.hpp"
#include "crust/cmp.hpp"
#include "crust/tuple.hpp"
#include "crust/utility.hpp"

#include "raii_checker.hpp"


using namespace crust;
using namespace cmp;


namespace {
struct C;

struct D;
} // namespace

namespace crust {
template <>
CRUST_IMPL_FOR(clone::Clone, C){};

template <>
CRUST_IMPL_FOR(clone::Clone, D){};
} // namespace crust

namespace {
struct A {};

struct B : A {};

struct C : test::RAIIChecker<C>, Impl<C, Trait<clone::Clone>> {
  CRUST_USE_BASE_CONSTRUCTORS(C, test::RAIIChecker<C>);
};

struct D : test::RAIIChecker<D>, Impl<D, Trait<clone::Clone>> {
  CRUST_USE_BASE_CONSTRUCTORS(D, test::RAIIChecker<D>);
};
} // namespace


GTEST_TEST(tuple, size_zero) {
  crust_static_assert(Require<Tuple<>, ZeroSizedType>::result);
  crust_static_assert(Require<Tuple<Tuple<>>, ZeroSizedType>::result);
  crust_static_assert(Require<Tuple<Tuple<>, Tuple<>>, ZeroSizedType>::result);
  crust_static_assert(
      Require<Tuple<Tuple<>, Tuple<>, Tuple<>>, ZeroSizedType>::result);

  crust_static_assert(sizeof(Tuple<>) == 1);
  crust_static_assert(sizeof(Tuple<Tuple<>>) == 1);
  crust_static_assert(sizeof(Tuple<Tuple<>, Tuple<>>) == 1);
  crust_static_assert(sizeof(Tuple<Tuple<>, Tuple<>, Tuple<>>) == 1);
  crust_static_assert(sizeof(Tuple<i32, Tuple<>, Tuple<>>) == sizeof(i32));
  crust_static_assert(sizeof(Tuple<Tuple<>, i32, Tuple<>>) == sizeof(i32));
  crust_static_assert(sizeof(Tuple<Tuple<>, Tuple<>, i32>) == sizeof(i32));

  constexpr auto empty_1 = make_tuple(make_tuple(), make_tuple(), make_tuple());
  crust_static_assert(empty_1.get<0>() == make_tuple());
  crust_static_assert(empty_1.get<1>() == make_tuple());
  crust_static_assert(empty_1.get<2>() == make_tuple());

  constexpr auto empty_2 = make_tuple(make_tuple(), 1);
  crust_static_assert(empty_2.get<0>() == make_tuple());
  crust_static_assert(empty_2.get<1>() == 1);

  crust_static_assert(Require<Tuple<>, PartialEq>::result);
  crust_static_assert(Require<Tuple<>, Eq>::result);

  crust_static_assert(std::is_trivially_copyable<Tuple<>>::value);

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
  crust_static_assert(!Require<Tuple<A>, ZeroSizedType>::result);

  crust_static_assert(!Require<Tuple<A>, PartialEq>::result);
  crust_static_assert(!Require<Tuple<A>, Eq>::result);

  crust_static_assert(std::is_trivially_copyable<Tuple<A>>::value);

  crust_static_assert(Require<Tuple<i32>, PartialEq>::result);
  crust_static_assert(Require<Tuple<i32>, Eq>::result);

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
  crust_static_assert(!Require<Tuple<A, B>, ZeroSizedType>::result);

  crust_static_assert(!Require<Tuple<A, B>, PartialEq>::result);
  crust_static_assert(!Require<Tuple<A, B>, Eq>::result);

  crust_static_assert(std::is_trivially_copyable<Tuple<A, B>>::value);

  crust_static_assert(Require<Tuple<i32, char>, PartialEq>::result);
  crust_static_assert(Require<Tuple<i32, char>, Eq>::result);

  auto tuple = make_tuple(0, 'a');

  EXPECT_TRUE(tuple.get<0>() == 0);
  EXPECT_TRUE(tuple.get<1>() == 'a');

  ++tuple.get<0>();
  ++tuple.get<1>();

  EXPECT_TRUE(std::get<0>(tuple) == 1);
  EXPECT_TRUE(std::get<1>(tuple) == 'b');

  crust_static_assert(make_tuple(true) != make_tuple(false));
  crust_static_assert(make_tuple(1) > make_tuple(0));
  crust_static_assert(make_tuple(0, 'b') > make_tuple(0, 'a'));

  EXPECT_TRUE(make_tuple(0, 1).cmp(make_tuple(0, 0)) == make_greater());
}

GTEST_TEST(tuple, raii) {
  auto recorder = std::make_shared<test::RAIIRecorder>(test::RAIIRecorder{});

  auto a = make_tuple(C{recorder}, D{recorder}, C{recorder}, D{recorder});
  auto b = a;
  auto c = clone::clone(a);
  c.clone_from(b);
  auto d = move(a);
}
