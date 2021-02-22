#include "gtest/gtest.h"

#include "utility.hpp"
#include "function.hpp"

#include "raii_checker.hpp"


using namespace crust;


void fn_c() {}

struct A : test::RAIIChecker<A> {
    CRUST_USE_BASE_CONSTRUCTORS_EXPLICIT(A, test::RAIIChecker<A>);

    void operator()() {}

    void operator()() const {}
};


GTEST_TEST(function, raii) {
    auto recorder = std::make_shared<test::RAIIRecorder>(test::RAIIRecorder{});

    {
        auto a = make_fn_mut<void()>(A{recorder});
        a();
    }

    {
        auto c = CRUST_MAKE_FN_MUT(fn_c);
        c();
    }

    {
        auto lambda = make_fn_mut([]() mutable {});
        lambda();
    }

    {
        DynFnMut<void()> dyn_b = make_dyn_fn_mut<void()>(A{recorder});
        dyn_b();
    }

    {
        DynFnMut<void()> dyn_c = CRUST_MAKE_DYN_FN_MUT(fn_c);
        dyn_c();
    }

    {
        DynFnMut<void()> lambda = make_dyn_fn_mut([]() mutable {});
        lambda();
    }

    {
        auto b = make_fn<void()>(A{recorder});
        b();
    }

    {
        auto c = CRUST_MAKE_FN(fn_c);
        c();
    }

    {
        auto lambda = make_fn([]() {});
        lambda();
    }

    {
        DynFn<void()> dyn_b = make_dyn_fn<void()>(A{recorder});
        dyn_b();
    }

    {
        DynFn<void()> dyn_c = CRUST_MAKE_DYN_FN(fn_c);
        dyn_c();
    }

    {
        DynFn<void()> lambda = make_dyn_fn([]() {});
        lambda();
    }
}
