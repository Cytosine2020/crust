#include "gtest/gtest.h"

#include "utility.hpp"
#include "enum.hpp"
#include "cmp.hpp"

#include "raii_checker.hpp"


using namespace crust;


namespace {
struct ClassA : test::RAIIChecker<ClassA> {
  CRUST_USE_BASE_CONSTRUCTORS(ClassA, test::RAIIChecker<ClassA>);
};

struct ClassB : test::RAIIChecker<ClassB> {
  CRUST_USE_BASE_CONSTRUCTORS(ClassB, test::RAIIChecker<ClassB>);
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

  crust_static_assert(!Derive<Enumerate, cmp::PartialEq>::result);
  crust_static_assert(!Derive<Enumerate, cmp::Eq>::result);

  auto recorder = std::make_shared<test::RAIIRecorder>(test::RAIIRecorder{});

  Enumerate a;
  a = Enumerate{ClassA{recorder}};
  EXPECT_TRUE(a.visit<bool>(VisitType<ClassA>{}));
  a = Enumerate{ClassB{recorder}};
  EXPECT_TRUE(a.visit<bool>(VisitType<ClassB>{}));
  a = ClassA{recorder};
  EXPECT_TRUE(a.visit<bool>(VisitType<ClassA>{}));
  a = ClassB{recorder};
  EXPECT_TRUE(a.visit<bool>(VisitType<ClassB>{}));

  Enumerate b;
  b = a;
  EXPECT_TRUE(b.visit<bool>(VisitType<ClassB>{}));
  b = move(a);

  Enumerate c{b};
  Enumerate d{move(c)};

  EXPECT_TRUE(d.visit<bool>(VisitType<ClassB>{}));
}


namespace {
struct A : public MonoStateTag {};

struct B : public MonoStateTag {};

struct C {};

struct D {};

struct E {};

struct F {};
}


GTEST_TEST(enum_, tag_only) {
  crust_static_assert(sizeof(Enum<A, B>) == sizeof(u32));
  crust_static_assert(sizeof(Enum<A, B, C, D, E, F>) > sizeof(u32));
}

GTEST_TEST(enum_, raii) {
  using Enumerate = Enum<A, B, C, D, E, F>;

  crust_static_assert(std::is_trivially_copyable<Enumerate>::value);
  crust_static_assert(std::is_standard_layout<Enumerate>::value);
  crust_static_assert(std::is_literal_type<Enumerate>::value);

  Enumerate a{A{}};
  EXPECT_TRUE(a.visit<bool>(VisitType<A>{}));
  a = B{};
  EXPECT_TRUE(a.visit<bool>(VisitType<B>{}));
  a = C{};
  EXPECT_TRUE(a.visit<bool>(VisitType<C>{}));
  a = D{};
  EXPECT_TRUE(a.visit<bool>(VisitType<D>{}));
  a = E{};
  EXPECT_TRUE(a.visit<bool>(VisitType<E>{}));
  a = F{};
  EXPECT_TRUE(a.visit<bool>(VisitType<F>{}));
}

GTEST_TEST(enum_, cmp) {
  using Enumerate = Enum<i32, char>;

  EXPECT_TRUE(Enumerate{0} == Enumerate{0});
  EXPECT_TRUE(Enumerate{0} != Enumerate{'a'});
  EXPECT_TRUE(Enumerate{0} != Enumerate{1});
}
