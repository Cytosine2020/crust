#include "gtest/gtest.h"

#include "utility.hpp"
#include "enum.hpp"
#include "cmp.hpp"

#include "raii_checker.hpp"

using namespace crust;


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

GTEST_TEST(enum_, enum_) {
    using Enumerate = Enum<ClassA, ClassB>;

    CRUST_STATIC_ASSERT(!CRUST_DERIVE(Enumerate, PartialEq));
    CRUST_STATIC_ASSERT(!CRUST_DERIVE(Enumerate, Eq));
    CRUST_STATIC_ASSERT(!std::is_trivially_copyable<Enumerate>::value);
    CRUST_STATIC_ASSERT(!std::is_literal_type<Enumerate>::value);

    auto recorder = std::make_shared<test::RAIIRecorder>(test::RAIIRecorder{});

    Enumerate a{ClassA{recorder}};
    EXPECT_TRUE((a.visit<VisitType<ClassA>, bool>()));
    a = Enumerate{ClassB{recorder}};
    EXPECT_TRUE((a.visit<VisitType<ClassB>, bool>()));
    a = ClassA{recorder};
    EXPECT_TRUE((a.visit<VisitType<ClassA>, bool>()));
    a = ClassB{recorder};
    EXPECT_TRUE((a.visit<VisitType<ClassB>, bool>()));
}


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


GTEST_TEST(enum_, raii) {
    using Enumerate = Enum<A, B, C, D, E, F>;

    CRUST_STATIC_ASSERT(std::is_trivially_copyable<Enumerate>::value);
    CRUST_STATIC_ASSERT(std::is_literal_type<Enumerate>::value);
    CRUST_STATIC_ASSERT(sizeof(Enum<A, B>) == sizeof(u32));
    CRUST_STATIC_ASSERT(sizeof(Enumerate) > sizeof(u32));

    Enumerate a{A{}};
    EXPECT_TRUE((a.visit<VisitType<A>, bool>()));
    a = B{};
    EXPECT_TRUE((a.visit<VisitType<B>, bool>()));
    a = C{};
    EXPECT_TRUE((a.visit<VisitType<C>, bool>()));
    a = D{};
    EXPECT_TRUE((a.visit<VisitType<D>, bool>()));
    a = E{};
    EXPECT_TRUE((a.visit<VisitType<E>, bool>()));
    a = F{};
    EXPECT_TRUE((a.visit<VisitType<F>, bool>()));
}

GTEST_TEST(enum_, cmp) {
    using Enumerate = Enum<i32, char>;

    EXPECT_TRUE(Enumerate{0} == Enumerate{0});
    EXPECT_TRUE(Enumerate{0} != Enumerate{'a'});
    EXPECT_TRUE(Enumerate{0} != Enumerate{1});
}
