#include "gtest/gtest.h"

#include "crust/cmp.hpp"
#include "crust/enum.hpp"
#include "crust/utility.hpp"

#include "raii_checker.hpp"


using namespace crust;


namespace {
template <class T>
struct VisitType {
  constexpr bool operator()(const T &) const { return true; }

  template <class U>
  constexpr bool operator()(const U &) const {
    return false;
  }
};

struct ClassA : test::RAIIChecker<ClassA> {
  CRUST_USE_BASE_CONSTRUCTORS(ClassA, test::RAIIChecker<ClassA>);
};

struct ClassB : test::RAIIChecker<ClassB> {
  CRUST_USE_BASE_CONSTRUCTORS(ClassB, test::RAIIChecker<ClassB>);
};

struct EnumA : Enum<ClassA, ClassB> {
  CRUST_ENUM_USE_BASE(EnumA, Enum<ClassA, ClassB>);
};
} // namespace


GTEST_TEST(enum_, enum_) {
  crust_static_assert(!Derive<EnumA, cmp::PartialEq>::result);
  crust_static_assert(!Derive<EnumA, cmp::Eq>::result);

  auto recorder = std::make_shared<test::RAIIRecorder>(test::RAIIRecorder{});

  EnumA a;
  a = EnumA{ClassA{recorder}};
  EXPECT_TRUE(a.visit<bool>(VisitType<ClassA>{}));
  a = EnumA{ClassB{recorder}};
  EXPECT_TRUE(a.visit<bool>(VisitType<ClassB>{}));
  a = ClassA{recorder};
  EXPECT_TRUE(a.visit<bool>(VisitType<ClassA>{}));
  a = ClassB{recorder};
  EXPECT_TRUE(a.visit<bool>(VisitType<ClassB>{}));

  EnumA b;
  b = a;
  EXPECT_TRUE(b.visit<bool>(VisitType<ClassB>{}));
  b = move(a);

  EnumA c{b};
  EnumA d{move(c)};

  EXPECT_TRUE(d.visit<bool>(VisitType<ClassB>{}));
}


namespace {
struct crust_ebco A :
    TupleStruct<>,
    AutoImpl<A, TupleStruct<>, ZeroSizedType, cmp::PartialEq, cmp::Eq> {
  CRUST_USE_BASE_CONSTRUCTORS(A, TupleStruct<>);
};
struct crust_ebco B :
    TupleStruct<>,
    AutoImpl<B, TupleStruct<>, ZeroSizedType, cmp::PartialEq, cmp::Eq> {
  CRUST_USE_BASE_CONSTRUCTORS(B, TupleStruct<>);
};
struct crust_ebco C :
    TupleStruct<i32>,
    AutoImpl<C, TupleStruct<i32>, cmp::PartialEq, cmp::Eq> {
  CRUST_USE_BASE_CONSTRUCTORS(C, TupleStruct<i32>);
};
struct crust_ebco D :
    TupleStruct<i32>,
    AutoImpl<D, TupleStruct<i32>, cmp::PartialEq, cmp::Eq> {
  CRUST_USE_BASE_CONSTRUCTORS(D, TupleStruct<i32>);
};
struct crust_ebco E :
    TupleStruct<i32>,
    AutoImpl<E, TupleStruct<i32>, cmp::PartialEq, cmp::Eq> {
  CRUST_USE_BASE_CONSTRUCTORS(E, TupleStruct<i32>);
};
struct crust_ebco F :
    TupleStruct<i32>,
    AutoImpl<F, TupleStruct<i32>, cmp::PartialEq, cmp::Eq> {
  CRUST_USE_BASE_CONSTRUCTORS(F, TupleStruct<i32>);
};

struct crust_ebco EnumB :
    Enum<A, B>,
    AutoImpl<EnumB, Enum<A, B>, cmp::PartialEq, cmp::Eq> {
  CRUST_ENUM_USE_BASE(EnumB, Enum<A, B>);
};

struct crust_ebco EnumC :
    Enum<A, B, C, D, E, F>,
    AutoImpl<EnumC, Enum<A, B, C, D, E, F>, cmp::PartialEq, cmp::Eq> {
  CRUST_ENUM_USE_BASE(EnumC, Enum<A, B, C, D, E, F>);
};
} // namespace


GTEST_TEST(enum_, tag_only) {
  crust_static_assert(sizeof(EnumB) == sizeof(u32));
  crust_static_assert(sizeof(EnumC) == 2 * sizeof(u32));
}

GTEST_TEST(enum_, raii) {
  crust_static_assert(std::is_trivially_copyable<EnumC>::value);
  crust_static_assert(std::is_standard_layout<EnumC>::value);
  crust_static_assert(std::is_literal_type<EnumC>::value);

  EnumC a;
  a = A{};
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

namespace {
struct crust_ebco EnumD :
    Enum<i32, char>,
    AutoImpl<EnumD, Enum<i32, char>, cmp::PartialEq, cmp::Eq> {
  CRUST_ENUM_USE_BASE(EnumD, Enum<i32, char>);
};
} // namespace

GTEST_TEST(enum_, cmp) {
  EXPECT_TRUE(EnumD{0} == EnumD{0});
  EXPECT_TRUE(EnumD{0} != EnumD{'a'});
  EXPECT_TRUE(EnumD{0} != EnumD{1});
}
