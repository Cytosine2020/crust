#include "gtest/gtest.h"

#include "crust/cmp.hpp"
#include "crust/enum.hpp"
#include "crust/utility.hpp"

#include "raii_checker.hpp"


using namespace crust;

namespace {
struct ClassA;

struct ClassB;
} // namespace

namespace crust {
template <>
CRUST_IMPL_FOR(clone::Clone, ClassA){};

template <>
CRUST_IMPL_FOR(clone::Clone, ClassB){};
} // namespace crust

namespace {
template <class T>
struct VisitType {
  constexpr bool operator()(const T &) const { return true; }

  template <class U>
  constexpr bool operator()(const U &) const {
    return false;
  }
};

struct ClassA : test::RAIIChecker<ClassA>, Impl<ClassA, Trait<clone::Clone>> {
  CRUST_USE_BASE_CONSTRUCTORS(ClassA, test::RAIIChecker<ClassA>);
};

struct ClassB : test::RAIIChecker<ClassB>, Impl<ClassB, Trait<clone::Clone>> {
  CRUST_USE_BASE_CONSTRUCTORS(ClassB, test::RAIIChecker<ClassB>);
};

struct EnumA :
    Enum<ClassA, ClassB>,
    Derive<EnumA, Enum<ClassA, ClassB>, clone::Clone> {
  CRUST_ENUM_USE_BASE(EnumA, Enum<ClassA, ClassB>);
};
} // namespace

GTEST_TEST(enum_, enum_) {
  crust_static_assert(!Require<EnumA, cmp::PartialEq>::result);
  crust_static_assert(!Require<EnumA, cmp::Eq>::result);

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

  a = ClassA{recorder};
  b = a.clone();
  EXPECT_TRUE(b.visit<bool>(VisitType<ClassA>{}));

  a = ClassB{recorder};
  b.clone_from(a);
  EXPECT_TRUE(b.visit<bool>(VisitType<ClassB>{}));

  b = move(a);
  EXPECT_TRUE(b.visit<bool>(VisitType<ClassB>{}));

  EnumA c{b};
  EXPECT_TRUE(b.visit<bool>(VisitType<ClassB>{}));

  EnumA d{move(c)};
  EXPECT_TRUE(d.visit<bool>(VisitType<ClassB>{}));
}

CRUST_ENUM_VARIANT(A);
CRUST_ENUM_VARIANT(B);
CRUST_ENUM_TUPLE_VARIANT(C, C, i32);
CRUST_ENUM_TUPLE_VARIANT(D, D, i32);
CRUST_ENUM_TUPLE_VARIANT(E, E, i32);
CRUST_ENUM_TUPLE_VARIANT(F, F, i32);

struct EnumB;
struct EnumC;
struct EnumD;
struct EnumE;

namespace crust {
template <>
struct NewDerive<EnumB> :
    DeriveInfo<
        EnumB,
        Enum<A, B>,
        Enum<cmp::Less, cmp::Equal, cmp::Greater>,
        Trait<cmp::PartialEq>,
        Trait<cmp::Eq>> {};

template <>
struct NewDerive<EnumC> :
    DeriveInfo<
        EnumC,
        Enum<A, B, C, D, E, F>,
        Enum<cmp::Less, cmp::Equal, cmp::Greater>,
        Trait<cmp::PartialEq>,
        Trait<cmp::Eq>> {};

template <>
struct NewDerive<EnumD> :
    DeriveInfo<
        EnumD,
        Enum<A, B>,
        Enum<cmp::Less, cmp::Equal, cmp::Greater>,
        Trait<cmp::PartialEq>,
        Trait<cmp::Eq>> {};

template <>
struct NewDerive<EnumE> :
    DeriveInfo<
        EnumE,
        Enum<A, B, C, D, E, F>,
        Enum<cmp::Less, cmp::Equal, cmp::Greater>,
        Trait<cmp::PartialEq>,
        Trait<cmp::Eq>> {};
} // namespace crust

struct crust_ebco EnumB : Enum<A, B>, AutoDerive<EnumB> {
  CRUST_ENUM_USE_BASE(EnumB, Enum<A, B>);
};

struct crust_ebco EnumC : Enum<A, B, C, D, E, F>, AutoDerive<EnumC> {
  CRUST_ENUM_USE_BASE(EnumC, Enum<A, B, C, D, E, F>);
};

struct crust_ebco EnumD : Enum<EnumRepr<i16>, A, B>, AutoDerive<EnumD> {
  CRUST_ENUM_USE_BASE(EnumD, Enum<EnumRepr<u16>, A, B>);
};

struct crust_ebco EnumE :
    Enum<EnumRepr<isize>, A, B, C, D, E, F>,
    AutoDerive<EnumE> {
  CRUST_ENUM_USE_BASE(EnumE, Enum<EnumRepr<isize>, A, B, C, D, E, F>);
};


GTEST_TEST(enum_, tag_only) {
  crust_static_assert(sizeof(EnumB) == sizeof(i32));
  crust_static_assert(sizeof(EnumC) == 2 * sizeof(i32));
  crust_static_assert(sizeof(EnumD) == sizeof(i16));
  crust_static_assert(sizeof(EnumE) == 2 * sizeof(isize));
}

GTEST_TEST(enum_, raii) {
  crust_static_assert(std::is_trivially_copyable<EnumC>::value);
  crust_static_assert(std::is_standard_layout<EnumC>::value);

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

struct EnumF;

namespace crust {
template <>
struct NewDerive<EnumF> :
    DeriveInfo<
        EnumF,
        Enum<i32, char>,
        Enum<cmp::Less, cmp::Equal, cmp::Greater>,
        Trait<cmp::PartialEq>,
        Trait<cmp::Eq>> {};
} // namespace crust


struct crust_ebco EnumF : Enum<i32, char>, AutoDerive<EnumF> {
  CRUST_ENUM_USE_BASE(EnumF, Enum<i32, char>);
};


GTEST_TEST(enum_, cmp) {
  EXPECT_TRUE(EnumF{0} == EnumF{0});
  EXPECT_TRUE(EnumF{0} != EnumF{'a'});
  EXPECT_TRUE(EnumF{0} != EnumF{1});
}
