#include "gtest/gtest.h"

#include "utility.hpp"
#include "function.hpp"

#include "raii_checker.hpp"


using namespace crust;


namespace {
struct A : test::RAIIChecker<A> {
  CRUST_USE_BASE_CONSTRUCTORS_EXPLICIT(A, test::RAIIChecker<A>);

  i32 operator()() { return 1; }

  i32 operator()() const { return 2; }

  i32 fn_a() { return 3; }

  i32 fn_b() const { return 4; }
};

i32 fn_c() { return 5; }

template<class F>
i32 test_fn(Fn<F, i32()> fn) { return fn(); }

template<class F>
i32 test_fn_mut(FnMut<F, i32()> fn) { return fn(); }

i32 test_dyn_fn(DynFn<i32()> fn) { return fn(); }

i32 test_dyn_fn_mut(DynFnMut<i32()> fn) { return fn(); }
}


GTEST_TEST(function, raii) {
  auto recorder = std::make_shared<test::RAIIRecorder>();

  A a{recorder};
  GTEST_ASSERT_EQ(bind(CRUST_TMPL_ARG(&A::fn_b))(a), 4);
  GTEST_ASSERT_EQ(bind_mut(CRUST_TMPL_ARG(&A::fn_a))(a), 3);

  GTEST_ASSERT_EQ(test_fn(bind(CRUST_TMPL_ARG(&fn_c))), 5);
  GTEST_ASSERT_EQ(test_fn(bind<i32()>(A{recorder})), 2);
  GTEST_ASSERT_EQ(test_fn(bind([]() { return 7; })), 7);

  GTEST_ASSERT_EQ(test_fn_mut(bind_mut(CRUST_TMPL_ARG(&fn_c))), 5);
  GTEST_ASSERT_EQ(test_fn_mut(bind_mut<i32()>(A{recorder})), 1);
  GTEST_ASSERT_EQ(test_fn_mut(bind_mut([]() mutable { return 6; })), 6);

  GTEST_ASSERT_EQ(test_dyn_fn(CRUST_TMPL_ARG(&fn_c)), 5);
  GTEST_ASSERT_EQ(test_dyn_fn(A{recorder}), 2);
  GTEST_ASSERT_EQ(test_dyn_fn([]() { return 9; }), 9);

  GTEST_ASSERT_EQ(test_dyn_fn_mut(CRUST_TMPL_ARG(&fn_c)), 5);
  GTEST_ASSERT_EQ(test_dyn_fn_mut(A{recorder}), 1);
  GTEST_ASSERT_EQ(test_dyn_fn_mut([]() mutable { return 8; }), 8);
}
