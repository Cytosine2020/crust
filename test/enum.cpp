#include "gtest/gtest.h"

#include "utility.hpp"
#include "enum_declare.hpp"
#include "cmp.hpp"

#include "raii_checker.hpp"

using namespace crust;

namespace {
struct ClassA : test::RAIIChecker<ClassA> {
    CRUST_USE_BASE_CONSTRUCTORS_EXPLICIT(ClassA, test::RAIIChecker<ClassA>);
};

struct ClassB : test::RAIIChecker<ClassA> {
    CRUST_USE_BASE_CONSTRUCTORS_EXPLICIT(ClassB, test::RAIIChecker<ClassA>);
};

template<class T>
struct VisitType {
    bool operator()(const T &) { return true; }

    template<class U>
    bool operator()(const U &) { return false; }
};
}


GTEST_TEST(enum_, enum_) {
    using Enumerate = Enum<ClassA, ClassB>;

    CRUST_STATIC_ASSERT(!Derive<Enumerate, cmp::PartialEq>::result);
    CRUST_STATIC_ASSERT(!Derive<Enumerate, cmp::Eq>::result);
    CRUST_STATIC_ASSERT(!std::is_trivially_copyable<Enumerate>::value);
    CRUST_STATIC_ASSERT(!std::is_literal_type<Enumerate>::value);

    auto recorder = std::make_shared<test::RAIIRecorder>(test::RAIIRecorder{});

    Enumerate a{ClassA{recorder}};
    EXPECT_TRUE((a.visit<bool>(VisitType<ClassA>{})));
    a = Enumerate{ClassB{recorder}};
    EXPECT_TRUE((a.visit<bool>(VisitType<ClassB>{})));
    a = ClassA{recorder};
    EXPECT_TRUE((a.visit<bool>(VisitType<ClassA>{})));
    a = ClassB{recorder};
    EXPECT_TRUE((a.visit<bool>(VisitType<ClassB>{})));
}


namespace {
struct A : public MonoStateTag {
};

struct B : public MonoStateTag {
};

struct C {
};

struct D {
};

struct E {
};

struct F {
};
}


GTEST_TEST(enum_, tag_only) {
    CRUST_STATIC_ASSERT(sizeof(Enum<A, B>) == sizeof(u32));
    CRUST_STATIC_ASSERT(sizeof(Enum<A, B, C, D, E, F>) > sizeof(u32));
}

GTEST_TEST(enum_, raii) {
    using Enumerate = Enum<A, B, C, D, E, F>;

    CRUST_STATIC_ASSERT(std::is_trivially_copyable<Enumerate>::value);
    CRUST_STATIC_ASSERT(std::is_literal_type<Enumerate>::value);

    Enumerate a{A{}};
    EXPECT_TRUE((a.visit<bool>(VisitType<A>{})));
    a = B{};
    EXPECT_TRUE((a.visit<bool>(VisitType<B>{})));
    a = C{};
    EXPECT_TRUE((a.visit<bool>(VisitType<C>{})));
    a = D{};
    EXPECT_TRUE((a.visit<bool>(VisitType<D>{})));
    a = E{};
    EXPECT_TRUE((a.visit<bool>(VisitType<E>{})));
    a = F{};
    EXPECT_TRUE((a.visit<bool>(VisitType<F>{})));
}

GTEST_TEST(enum_, cmp) {
    using Enumerate = Enum<i32, char>;

    EXPECT_TRUE(Enumerate{0} == Enumerate{0});
    EXPECT_TRUE(Enumerate{0} != Enumerate{'a'});
    EXPECT_TRUE(Enumerate{0} != Enumerate{1});
}
