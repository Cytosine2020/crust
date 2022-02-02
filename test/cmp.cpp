#include "gtest/gtest.h"

#include "utility.hpp"
#include "cmp.hpp"


using namespace crust;
using namespace cmp;


GTEST_TEST(cmp, derive) {
  CRUST_STATIC_ASSERT(Derive<bool, PartialEq>::result);
  CRUST_STATIC_ASSERT(Derive<u8, PartialEq>::result);
  CRUST_STATIC_ASSERT(Derive<i8, PartialEq>::result);
  CRUST_STATIC_ASSERT(Derive<u16, PartialEq>::result);
  CRUST_STATIC_ASSERT(Derive<i16, PartialEq>::result);
  CRUST_STATIC_ASSERT(Derive<u32, PartialEq>::result);
  CRUST_STATIC_ASSERT(Derive<i32, PartialEq>::result);
  CRUST_STATIC_ASSERT(Derive<u64, PartialEq>::result);
  CRUST_STATIC_ASSERT(Derive<i64, PartialEq>::result);
  CRUST_STATIC_ASSERT(Derive<usize, PartialEq>::result);
  CRUST_STATIC_ASSERT(Derive<isize, PartialEq>::result);

  CRUST_STATIC_ASSERT(Derive<bool, Eq>::result);
  CRUST_STATIC_ASSERT(Derive<u8, Eq>::result);
  CRUST_STATIC_ASSERT(Derive<i8, Eq>::result);
  CRUST_STATIC_ASSERT(Derive<u16, Eq>::result);
  CRUST_STATIC_ASSERT(Derive<i16, Eq>::result);
  CRUST_STATIC_ASSERT(Derive<u32, Eq>::result);
  CRUST_STATIC_ASSERT(Derive<i32, Eq>::result);
  CRUST_STATIC_ASSERT(Derive<u64, Eq>::result);
  CRUST_STATIC_ASSERT(Derive<i64, Eq>::result);
  CRUST_STATIC_ASSERT(Derive<usize, Eq>::result);
  CRUST_STATIC_ASSERT(Derive<isize, Eq>::result);
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
  EXPECT_EQ(make_equal().then_with(bind([]() {
    return make_less();
  })), make_less());
  EXPECT_EQ(make_less().then_with(bind([]() {
    return make_equal();
  })), make_less());
  EXPECT_EQ(make_greater().then_with(bind([]() {
    return make_equal();
  })), make_greater());
}


GTEST_TEST(cmp, min_max) {
  EXPECT_EQ(make_tuple(0, 'b'), min(make_tuple(0, 'b'), make_tuple(1, 'a')));
  EXPECT_EQ(make_tuple(1, 'a'), max(make_tuple(0, 'b'), make_tuple(1, 'a')));

  EXPECT_EQ(make_tuple(0, 'b'), min_by_key(
      make_tuple(0, 'b'),
      make_tuple(1, 'a'),
      bind([](const Tuple<i32, char> &value) {
        return value.get<0>();
      })
  ));

  EXPECT_EQ(make_tuple(1, 'a'), min_by_key(
      make_tuple(0, 'b'),
      make_tuple(1, 'a'),
      bind([](const Tuple<i32, char> &value) {
        return value.get<1>();
      })
  ));

  EXPECT_EQ(make_tuple(1, 'a'), max_by_key(
      make_tuple(0, 'b'),
      make_tuple(1, 'a'),
      bind([](const Tuple<i32, char> &value) {
        return value.get<0>();
      })
  ));

  EXPECT_EQ(make_tuple(0, 'b'), max_by_key(
      make_tuple(0, 'b'),
      make_tuple(1, 'a'),
      bind([](const Tuple<i32, char> &value) {
        return value.get<1>();
      })
  ));
}
