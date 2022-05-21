#include "gtest/gtest.h"

#include "crust/clone.hpp"
#include "crust/cmp.hpp"
#include "crust/tuple.hpp"
#include "crust/utility.hpp"

#include "raii_checker.hpp"


#include <tuple>

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
  // crust_static_assert(sizeof(Tuple<Tuple<>, Tuple<>, Tuple<Tuple<>>>) == 1);
  // crust_static_assert(sizeof(Tuple<Tuple<>, Tuple<Tuple<>>, Tuple<>>) == 1);
  // crust_static_assert(sizeof(Tuple<Tuple<Tuple<>>, Tuple<>, Tuple<>>) == 1);
  // crust_static_assert(
  //     sizeof(Tuple<Tuple<>, Tuple<Tuple<>>, Tuple<Tuple<Tuple<>>>>) == 1);
  // crust_static_assert(
  //     sizeof(Tuple<Tuple<>, Tuple<Tuple<Tuple<>>>, Tuple<Tuple<>>>) == 1);
  // crust_static_assert(
  //     sizeof(Tuple<Tuple<Tuple<>>, Tuple<>, Tuple<Tuple<Tuple<>>>>) == 1);
  // crust_static_assert(
  //     sizeof(Tuple<Tuple<Tuple<>>, Tuple<Tuple<Tuple<>>>, Tuple<>>) == 1);
  // crust_static_assert(
  //     sizeof(Tuple<Tuple<Tuple<Tuple<>>>, Tuple<>, Tuple<Tuple<>>>) == 1);
  // crust_static_assert(
  //     sizeof(Tuple<Tuple<Tuple<Tuple<>>>, Tuple<Tuple<>>, Tuple<>>) == 1);
  crust_static_assert(sizeof(Tuple<i32, Tuple<>, Tuple<>>) == sizeof(i32));
  crust_static_assert(sizeof(Tuple<Tuple<>, i32, Tuple<>>) == sizeof(i32));
  crust_static_assert(sizeof(Tuple<Tuple<>, Tuple<>, i32>) == sizeof(i32));

  constexpr auto empty_1 = tuple(tuple(), tuple(), tuple());
  crust_static_assert(empty_1.get<0>() == tuple());
  crust_static_assert(empty_1.get<1>() == tuple());
  crust_static_assert(empty_1.get<2>() == tuple());

  constexpr auto empty_2 = tuple(tuple(), 1);
  crust_static_assert(empty_2.get<0>() == tuple());
  crust_static_assert(empty_2.get<1>() == 1);

  constexpr auto empty_3 = tuple(tuple(), tuple(tuple()));
  crust_static_assert(empty_3.get<0>() == tuple());
  // crust_static_assert(empty_3.get<1>() == tuple(tuple()));
  crust_static_assert(empty_3.get<1>().get<0>() == tuple());

  constexpr auto empty_4 = tuple(tuple(tuple()), tuple());
  // crust_static_assert(empty_4.get<0>() == tuple(tuple()));
  crust_static_assert(empty_4.get<1>() == tuple());
  crust_static_assert(empty_4.get<0>().get<0>() == tuple());

  crust_static_assert(Require<Tuple<>, PartialEq>::result);
  crust_static_assert(Require<Tuple<>, Eq>::result);

  crust_static_assert(std::is_trivially_copyable<Tuple<>>::value);

  crust_static_assert(tuple() == tuple());
  crust_static_assert(!(tuple() != tuple()));
  crust_static_assert(!(tuple() < tuple()));
  crust_static_assert(tuple() <= tuple());
  crust_static_assert(!(tuple() > tuple()));
  crust_static_assert(tuple() >= tuple());
  EXPECT_EQ(operator_partial_cmp(tuple(), tuple()), make_some(make_equal()));
  EXPECT_EQ(operator_cmp(tuple(), tuple()), make_equal());
}

GTEST_TEST(tuple, size_one) {
  crust_static_assert(!Require<Tuple<A>, ZeroSizedType>::result);

  crust_static_assert(!Require<Tuple<A>, PartialEq>::result);
  crust_static_assert(!Require<Tuple<A>, Eq>::result);

  crust_static_assert(std::is_trivially_copyable<Tuple<A>>::value);

  crust_static_assert(Require<Tuple<i32>, PartialEq>::result);
  crust_static_assert(Require<Tuple<i32>, Eq>::result);

  crust_static_assert(tuple(0) == tuple(0));
  crust_static_assert(!(tuple(0) != tuple(0)));
  crust_static_assert(!(tuple(0) < tuple(0)));
  crust_static_assert(tuple(0) <= tuple(0));
  crust_static_assert(!(tuple(0) > tuple(0)));
  crust_static_assert(tuple(0) >= tuple(0));
  EXPECT_EQ(operator_partial_cmp(tuple(0), tuple(0)), make_some(make_equal()));
  EXPECT_EQ(operator_cmp(tuple(0), tuple(0)), make_equal());

  auto value = tuple(0);

  EXPECT_EQ(value.get<0>(), 0);

  ++value.get<0>();

  EXPECT_EQ(value.get<0>(), 1);
}

GTEST_TEST(tuple, size_two) {
  crust_static_assert(!Require<Tuple<A, B>, ZeroSizedType>::result);

  crust_static_assert(!Require<Tuple<A, B>, PartialEq>::result);
  crust_static_assert(!Require<Tuple<A, B>, Eq>::result);

  crust_static_assert(std::is_trivially_copyable<Tuple<A, B>>::value);

  crust_static_assert(Require<Tuple<i32, char>, PartialEq>::result);
  crust_static_assert(Require<Tuple<i32, char>, Eq>::result);

  auto value = tuple(0, 'a');

  EXPECT_TRUE(value.get<0>() == 0);
  EXPECT_TRUE(value.get<1>() == 'a');

  ++value.get<0>();
  ++value.get<1>();

  EXPECT_TRUE(std::get<0>(value) == 1);
  EXPECT_TRUE(std::get<1>(value) == 'b');

  crust_static_assert(tuple(true) != tuple(false));
  crust_static_assert(tuple(1) > tuple(0));
  crust_static_assert(tuple(0, 'b') > tuple(0, 'a'));

  EXPECT_TRUE(tuple(0, 1).cmp(tuple(0, 0)) == make_greater());
}

GTEST_TEST(tuple, raii) {
  auto recorder = std::make_shared<test::RAIIRecorder>(test::RAIIRecorder{});

  auto a = tuple(C{recorder}, D{recorder}, C{recorder}, D{recorder});
  auto b = a;
  auto c = clone::clone(a);
  c.clone_from(b);
  auto d = move(a);
}

GTEST_TEST(tuple, tie) {
  const auto tuple2 = tuple(1, 2);
  const auto tuple7 = tuple(1, 2, 3, 4, 5, 6, 7);

  int a, b;
  tie(a, b) = tuple(1, 2);

  EXPECT_EQ(a, 1);
  EXPECT_EQ(b, 2);

  Ref<int> c, d;
  tie(c, d) = tuple2;

  EXPECT_EQ(*c, 1);
  EXPECT_EQ(*d, 2);

  int e, f;
  tie(e, _) = tuple(1, 2);
  tie(_, f) = tuple(1, 2);

  EXPECT_EQ(e, 1);
  EXPECT_EQ(f, 2);

  int g, h;
  Ref<int> i, j;
  tie(g, i, _, __, h, j) = tuple7;

  EXPECT_EQ(g, 1);
  EXPECT_EQ(*i, 2);
  EXPECT_EQ(h, 6);
  EXPECT_EQ(*j, 7);

  int k, l;
  tie(k, __) = tuple(1, 2, 3, 4, 5, 6, 7);
  tie(__, l) = tuple(1, 2, 3, 4, 5, 6, 7);

  EXPECT_EQ(k, 1);
  EXPECT_EQ(l, 7);

  int m, n, o, p;
  tie(tie(m, n), tie(o, p)) = tuple(tuple(1, 2), tuple(3, 4));

  EXPECT_EQ(m, 1);
  EXPECT_EQ(n, 2);
  EXPECT_EQ(o, 3);
  EXPECT_EQ(p, 4);

#if __cplusplus > 201402L
  auto [a1, b1] = tuple(1, 2);

  EXPECT_EQ(a1, 1);
  EXPECT_EQ(b1, 2);
#endif
}
