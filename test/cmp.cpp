#include "gtest/gtest.h"

#include "crust/cmp.hpp"
#include "crust/tuple.hpp"
#include "crust/utility.hpp"


using namespace crust;
using namespace cmp;
using ops::bind;


GTEST_TEST(cmp, derive) {
  crust_static_assert(Require<bool, PartialEq>::result);
  crust_static_assert(Require<char, PartialEq>::result);
  crust_static_assert(Require<u8, PartialEq>::result);
  crust_static_assert(Require<i8, PartialEq>::result);
  crust_static_assert(Require<u16, PartialEq>::result);
  crust_static_assert(Require<i16, PartialEq>::result);
  crust_static_assert(Require<u32, PartialEq>::result);
  crust_static_assert(Require<i32, PartialEq>::result);
  crust_static_assert(Require<u64, PartialEq>::result);
  crust_static_assert(Require<i64, PartialEq>::result);
  crust_static_assert(Require<usize, PartialEq>::result);
  crust_static_assert(Require<isize, PartialEq>::result);

  crust_static_assert(Require<bool, Eq>::result);
  crust_static_assert(Require<char, Eq>::result);
  crust_static_assert(Require<u8, Eq>::result);
  crust_static_assert(Require<i8, Eq>::result);
  crust_static_assert(Require<u16, Eq>::result);
  crust_static_assert(Require<i16, Eq>::result);
  crust_static_assert(Require<u32, Eq>::result);
  crust_static_assert(Require<i32, Eq>::result);
  crust_static_assert(Require<u64, Eq>::result);
  crust_static_assert(Require<i64, Eq>::result);
  crust_static_assert(Require<usize, Eq>::result);
  crust_static_assert(Require<isize, Eq>::result);

  crust_static_assert(Require<bool, PartialOrd>::result);
  crust_static_assert(Require<char, PartialOrd>::result);
  crust_static_assert(Require<u8, PartialOrd>::result);
  crust_static_assert(Require<i8, PartialOrd>::result);
  crust_static_assert(Require<u16, PartialOrd>::result);
  crust_static_assert(Require<i16, PartialOrd>::result);
  crust_static_assert(Require<u32, PartialOrd>::result);
  crust_static_assert(Require<i32, PartialOrd>::result);
  crust_static_assert(Require<u64, PartialOrd>::result);
  crust_static_assert(Require<i64, PartialOrd>::result);
  crust_static_assert(Require<usize, PartialOrd>::result);
  crust_static_assert(Require<isize, PartialOrd>::result);

  crust_static_assert(Require<bool, Ord>::result);
  crust_static_assert(Require<char, Ord>::result);
  crust_static_assert(Require<u8, Ord>::result);
  crust_static_assert(Require<i8, Ord>::result);
  crust_static_assert(Require<u16, Ord>::result);
  crust_static_assert(Require<i16, Ord>::result);
  crust_static_assert(Require<u32, Ord>::result);
  crust_static_assert(Require<i32, Ord>::result);
  crust_static_assert(Require<u64, Ord>::result);
  crust_static_assert(Require<i64, Ord>::result);
  crust_static_assert(Require<usize, Ord>::result);
  crust_static_assert(Require<isize, Ord>::result);
}


GTEST_TEST(cmp, ordering) {
  crust_static_assert(make_less() == make_less());
  crust_static_assert(make_less() == make_less());
  crust_static_assert(make_equal() == make_equal());
  crust_static_assert(make_greater() == make_greater());
  EXPECT_TRUE(make_less() < make_equal());
  EXPECT_TRUE(make_less() < make_greater());
  EXPECT_TRUE(make_equal() < make_greater());

  EXPECT_EQ(make_equal().then(make_less()), make_less());
  EXPECT_EQ(make_less().then(make_equal()), make_less());
  EXPECT_EQ(make_greater().then(make_equal()), make_greater());
  EXPECT_EQ(
      make_equal().then_with(bind([]() { return make_less(); })), make_less());
  EXPECT_EQ(
      make_less().then_with(bind([]() { return make_equal(); })), make_less());
  EXPECT_EQ(
      make_greater().then_with(bind([]() { return make_equal(); })),
      make_greater());
}


GTEST_TEST(cmp, min_max) {
  EXPECT_EQ(make_tuple(0, 'b'), min(make_tuple(0, 'b'), make_tuple(1, 'a')));
  EXPECT_EQ(make_tuple(1, 'a'), max(make_tuple(0, 'b'), make_tuple(1, 'a')));

  EXPECT_EQ(
      make_tuple(0, 'b'),
      min_by_key(
          make_tuple(0, 'b'),
          make_tuple(1, 'a'),
          bind([](const Tuple<i32, char> &value) { return value.get<0>(); })));

  EXPECT_EQ(
      make_tuple(1, 'a'),
      min_by_key(
          make_tuple(0, 'b'),
          make_tuple(1, 'a'),
          bind([](const Tuple<i32, char> &value) { return value.get<1>(); })));

  EXPECT_EQ(
      make_tuple(1, 'a'),
      max_by_key(
          make_tuple(0, 'b'),
          make_tuple(1, 'a'),
          bind([](const Tuple<i32, char> &value) { return value.get<0>(); })));

  EXPECT_EQ(
      make_tuple(0, 'b'),
      max_by_key(
          make_tuple(0, 'b'),
          make_tuple(1, 'a'),
          bind([](const Tuple<i32, char> &value) { return value.get<1>(); })));
}

GTEST_TEST(cmp, reverse) {
  crust_static_assert(sizeof(cmp::Reverse<i32>) == sizeof(i32));
  crust_static_assert(Require<cmp::Reverse<i32>, PartialEq>::result);
  crust_static_assert(Require<cmp::Reverse<i32>, Eq>::result);
  crust_static_assert(Require<cmp::Reverse<i32>, PartialOrd>::result);
  crust_static_assert(Require<cmp::Reverse<i32>, Ord>::result);

  cmp::Reverse<i32> a{0};
  cmp::Reverse<i32> b{1};

  EXPECT_TRUE(a > b);
}
