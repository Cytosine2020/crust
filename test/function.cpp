#include "gtest/gtest.h"

#include "ops/function.hpp"
#include "utility.hpp"

#include "raii_checker.hpp"


using namespace crust;
using ops::bind;
using ops::bind_mut;


namespace {
struct A : test::RAIIChecker<A> {
  CRUST_USE_BASE_CONSTRUCTORS(A, test::RAIIChecker<A>);

  i32 operator()() { return 1; }

  i32 operator()() const { return 2; }

  i32 fn_a() { return 3; }

  i32 fn_b() const { return 4; }
};

i32 fn_c() { return 5; }

i32 fn_d(i32 a) { return a; }

i32 fn_e(i32 &a) { return a; }

i32 fn_f(i32 &&a) { return a; }

template <class F>
i32 test_fn(ops::Fn<F, i32()> fn) {
  return fn();
}

template <class F>
i32 test_fn_mut(ops::FnMut<F, i32()> fn) {
  return fn();
}

i32 test_dyn_fn(ops::DynFn<i32()> fn) { return fn(); }

i32 test_dyn_fn_mut(ops::DynFnMut<i32()> fn) { return fn(); }
} // namespace

GTEST_TEST(function, raii) {
  auto recorder = std::make_shared<test::RAIIRecorder>();

  GTEST_ASSERT_EQ(bind(crust_tmpl_arg(&fn_d))(11), 11);
  GTEST_ASSERT_EQ(bind(crust_tmpl_arg(&fn_f))(13), 13);

  i32 i = 14;
  GTEST_ASSERT_EQ(bind(crust_tmpl_arg(&fn_d))(i), 14);
  GTEST_ASSERT_EQ(bind(crust_tmpl_arg(&fn_e))(i), 14);

  A a{recorder};
  GTEST_ASSERT_EQ(bind(crust_tmpl_arg(&A::fn_b))(a), 4);
  GTEST_ASSERT_EQ(bind_mut(crust_tmpl_arg(&A::fn_a))(a), 3);

  GTEST_ASSERT_EQ(test_fn(bind(crust_tmpl_arg(&fn_c))), 5);
  GTEST_ASSERT_EQ(test_fn(bind<i32()>(A{recorder})), 2);
  GTEST_ASSERT_EQ(test_fn(bind([]() { return 7; })), 7);

  GTEST_ASSERT_EQ(test_fn_mut(bind_mut(crust_tmpl_arg(&fn_c))), 5);
  GTEST_ASSERT_EQ(test_fn_mut(bind_mut<i32()>(A{recorder})), 1);
  GTEST_ASSERT_EQ(test_fn_mut(bind_mut([]() mutable { return 6; })), 6);

  GTEST_ASSERT_EQ(test_dyn_fn(crust_tmpl_arg(&fn_c)), 5);
  GTEST_ASSERT_EQ(test_dyn_fn(A{recorder}), 2);
  GTEST_ASSERT_EQ(test_dyn_fn([]() { return 9; }), 9);

  GTEST_ASSERT_EQ(test_dyn_fn_mut(crust_tmpl_arg(&fn_c)), 5);
  GTEST_ASSERT_EQ(test_dyn_fn_mut(A{recorder}), 1);
  GTEST_ASSERT_EQ(test_dyn_fn_mut([]() mutable { return 8; }), 8);
}
