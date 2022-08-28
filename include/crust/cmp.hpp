#ifndef CRUST_CMP_HPP
#define CRUST_CMP_HPP


#include "cmp_decl.hpp"

#include "crust/enum_decl.hpp"
#include "crust/option.hpp"
#include "crust/utility.hpp"


namespace crust {
namespace cmp {
struct Less;
struct Equal;
struct Greater;
struct Ordering;
} // namespace cmp

template <>
struct BluePrint<cmp::Less> : TmplType<TupleStruct<>> {};

template <>
struct BluePrint<cmp::Equal> : TmplType<TupleStruct<>> {};

template <>
struct BluePrint<cmp::Greater> : TmplType<TupleStruct<>> {};

template <>
struct BluePrint<cmp::Ordering> :
    TmplType<Enum<cmp::Less, cmp::Equal, cmp::Greater>> {};

namespace cmp {
CRUST_DISCRIMINANT_VARIANT(Less, -1);
CRUST_DISCRIMINANT_VARIANT(Equal, 0);
CRUST_DISCRIMINANT_VARIANT(Greater, 1);

struct crust_ebco Ordering :
    Enum<EnumRepr<i8>, Less, Equal, Greater>,
    Derive<
        Ordering,
        Trait<cmp::PartialEq>,
        Trait<cmp::Eq>,
        Trait<cmp::PartialOrd>,
        Trait<cmp::Ord>> {
  CRUST_ENUM_USE_BASE(Ordering, Enum<EnumRepr<i8>, Less, Equal, Greater>);

  crust_cxx17_constexpr Ordering reverse() const {
    return this->template visit<Ordering>(
        [](const Less &) { return Greater{}; },
        [](const Equal &) { return Equal{}; },
        [](const Greater &) { return Less{}; });
  }

  crust_cxx17_constexpr Ordering then(const Ordering &other) const {
    return this->template visit<Ordering>(
        [](const Less &) { return Less{}; },
        [&](const Equal &) { return other; },
        [](const Greater &) { return Greater{}; });
  }

  template <class F>
  constexpr Ordering then_with(ops::Fn<F, Ordering()> f) const {
    return this->template visit<Ordering>(
        [](const Less &) { return Less{}; },
        [&](const Equal &) { return f(); },
        [](const Greater &) { return Greater{}; });
  }
};

crust_always_inline constexpr Ordering make_less() { return Less{}; }

crust_always_inline constexpr Ordering make_equal() { return Equal{}; }

crust_always_inline constexpr Ordering make_greater() { return Greater{}; }
} // namespace cmp

/// because c++ do not support add member function for primitive types,
/// following two functions are used for invoking `partial_cmp' or `cmp' for
/// both struct implemented `PartialOrd' or `Ord' trait and primitive types.
template <class T, class U>
crust_always_inline constexpr Option<cmp::Ordering>
operator_partial_cmp(const T &v1, const U &v2) {
  return v1.partial_cmp(v2);
}

template <class T>
crust_always_inline constexpr cmp::Ordering
operator_cmp(const T &v1, const T &v2) {
  return v1.cmp(v2);
}

#define _IMPL_PRIMITIVE(FN, ...)                                               \
  template <>                                                                  \
  FN(bool, ##__VA_ARGS__);                                                     \
  template <>                                                                  \
  FN(char, ##__VA_ARGS__);                                                     \
  template <>                                                                  \
  FN(u8, ##__VA_ARGS__);                                                       \
  template <>                                                                  \
  FN(i8, ##__VA_ARGS__);                                                       \
  template <>                                                                  \
  FN(u16, ##__VA_ARGS__);                                                      \
  template <>                                                                  \
  FN(i16, ##__VA_ARGS__);                                                      \
  template <>                                                                  \
  FN(u32, ##__VA_ARGS__);                                                      \
  template <>                                                                  \
  FN(i32, ##__VA_ARGS__);                                                      \
  template <>                                                                  \
  FN(u64, ##__VA_ARGS__);                                                      \
  template <>                                                                  \
  FN(i64, ##__VA_ARGS__);                                                      \
  template <class T>                                                           \
  FN(T *, ##__VA_ARGS__)

#define _IMPL_OPERATOR_CMP(type, ...)                                          \
  crust_always_inline constexpr cmp::Ordering operator_cmp(                    \
      const type &v1, const type &v2) {                                        \
    return v1 < v2 ? cmp::make_less() :                                        \
        v1 > v2    ? cmp::make_greater() :                                     \
                     cmp::make_equal();                                           \
  }

_IMPL_PRIMITIVE(_IMPL_OPERATOR_CMP);

#undef _IMPL_OPERATOR_CMP

#define _IMPL_OPERATOR_PARTIAL_CMP(type, ...)                                  \
  crust_always_inline constexpr Option<cmp::Ordering> operator_partial_cmp(    \
      const type &v1, const type &v2) {                                        \
    return make_some(operator_cmp(v1, v2));                                    \
  }

_IMPL_PRIMITIVE(_IMPL_OPERATOR_PARTIAL_CMP);

#undef _IMPL_OPERATOR_PARTIAL_CMP

namespace cmp {
// todo: refactor
template <class Self, class Rhs>
constexpr bool PartialOrd<Self, Rhs>::lt(const Rhs &other) const {
  return operator_partial_cmp(self(), other) == make_some(make_less());
}

template <class Self, class Rhs>
constexpr bool PartialOrd<Self, Rhs>::le(const Rhs &other) const {
  return operator_partial_cmp(self(), other) == make_some(make_greater()) ||
      operator_partial_cmp(self(), other) == make_some(make_equal());
}

template <class Self, class Rhs>
constexpr bool PartialOrd<Self, Rhs>::gt(const Rhs &other) const {
  return operator_partial_cmp(self(), other) == make_some(make_greater());
}

template <class Self, class Rhs>
constexpr bool PartialOrd<Self, Rhs>::ge(const Rhs &other) const {
  return operator_partial_cmp(self(), other) == make_some(make_greater()) ||
      operator_partial_cmp(self(), other) == make_some(make_equal());
}

template <class T>
constexpr T min(T &&v1, T &&v2) {
  crust_static_assert(Require<T, Ord>::result);
  return move(v1).min(forward<T>(v2));
}

template <class T, class F>
constexpr T
min_by(T &&v1, T &&v2, ops::Fn<F, Ordering(const T &, const T &)> compare) {
  crust_static_assert(Require<T, Ord>::result);
  return compare(v1, v2) == make_greater() ? forward<T>(v2) : forward<T>(v1);
}

template <class T, class F, class K>
constexpr T min_by_key(T &&v1, T &&v2, ops::Fn<F, K(const T &)> f) {
  crust_static_assert(Require<T, Ord>::result);
  return min_by(
      forward<T>(v1), forward<T>(v2), ops::bind([&](const T &v1, const T &v2) {
        return operator_cmp(f(v1), f(v2));
      }));
}

template <class T>
constexpr T max(T &&v1, T &&v2) {
  crust_static_assert(Require<T, Ord>::result);
  return move(v1).max(forward<T>(v2));
}

template <class T, class F>
constexpr T
max_by(T &&v1, T &&v2, ops::Fn<F, Ordering(const T &, const T &)> compare) {
  crust_static_assert(Require<T, Ord>::result);
  return compare(v1, v2) == make_greater() ? forward<T>(v1) : forward<T>(v2);
}

template <class T, class F, class K>
constexpr T max_by_key(T &&v1, T &&v2, ops::Fn<F, K(const T &)> f) {
  crust_static_assert(Require<T, Ord>::result);
  return max_by(
      forward<T>(v1), forward<T>(v2), ops::bind([&](const T &v1, const T &v2) {
        return operator_cmp(f(v1), f(v2));
      }));
}

template <class T>
struct crust_ebco Reverse :
    TupleStruct<T>,
    Derive<Reverse<T>, Trait<ZeroSizedType>>,
    Impl<
        Reverse<T>,
        Trait<PartialEq>,
        Trait<Eq>,
        Trait<PartialOrd>,
        Trait<Ord>> {
  CRUST_USE_BASE_CONSTRUCTORS(Reverse, TupleStruct<T>);
};
} // namespace cmp

template <class T>
CRUST_IMPL_FOR(cmp::PartialEq, cmp::Reverse<T>, Require<T, cmp::PartialEq>) {
  CRUST_IMPL_USE_SELF(cmp::Reverse<T>);

  constexpr bool eq(const Self &other) const {
    return other.template get<0>() == self().template get<0>();
  }
};

template <class T>
CRUST_IMPL_FOR(cmp::Eq, cmp::Reverse<T>, Require<T, cmp::Eq>){};

template <class T>
CRUST_IMPL_FOR(cmp::PartialOrd, cmp::Reverse<T>, Require<T, cmp::PartialOrd>) {
  CRUST_IMPL_USE_SELF(cmp::Reverse<T>);

  constexpr Option<cmp::Ordering> partial_cmp(const Self &other) const {
    return operator_partial_cmp(
        other.template get<0>(), self().template get<0>());
  }

  constexpr bool lt(const Self &other) const {
    return other.template get<0>() < self().template get<0>();
  }

  constexpr bool le(const Self &other) const {
    return other.template get<0>() <= self().template get<0>();
  }

  constexpr bool gt(const Self &other) const {
    return other.template get<0>() > self().template get<0>();
  }

  constexpr bool ge(const Self &other) const {
    return other.template get<0>() >= self().template get<0>();
  }
};

template <class T>
CRUST_IMPL_FOR(cmp::Ord, cmp::Reverse<T>, Require<T, cmp::Ord>) {
  CRUST_IMPL_USE_SELF(cmp::Reverse<T>);

  constexpr cmp::Ordering cmp(const Self &other) const {
    return operator_cmp(other.template get<0>(), self().template get<0>());
  }
};

#define _DERIVE_PRIMITIVE(PRIMITIVE, TRAIT, ...)                               \
  struct Require<PRIMITIVE, TRAIT, ##__VA_ARGS__> : BoolVal<true> {}

_IMPL_PRIMITIVE(_DERIVE_PRIMITIVE, cmp::PartialEq);
_IMPL_PRIMITIVE(_DERIVE_PRIMITIVE, cmp::Eq);
_IMPL_PRIMITIVE(_DERIVE_PRIMITIVE, cmp::PartialOrd);
_IMPL_PRIMITIVE(_DERIVE_PRIMITIVE, cmp::Ord);

#undef _DERIVE_PRIMITIVE
#undef _IMPL_PRIMITIVE

// todo: implement for float point numbers

namespace _impl_derive {
template <class Self, class Base, usize rev_index>
constexpr Option<cmp::Ordering>
TupleLikePartialOrdHelper<Self, Base, rev_index>::partial_cmp(
    const Self &a, const Self &b) {
  return make_some(operator_cmp(a, b)); // FIXME: temporary implement
}

template <class Self, class Base>
constexpr Option<cmp::Ordering>
TupleLikePartialOrdHelper<Self, Base, 0>::partial_cmp(
    const Self &, const Self &) {
  return make_some(cmp::make_equal());
}

template <class Self, class Base, usize rev_index>
constexpr cmp::Ordering
TupleLikeOrdHelper<Self, Base, rev_index>::cmp(const Self &a, const Self &b) {
  return operator_cmp(Getter::get(a), Getter::get(b)).then(Remains::cmp(a, b));
}

template <class Self, class Base>
constexpr cmp::Ordering
TupleLikeOrdHelper<Self, Base, 0>::cmp(const Self &, const Self &) {
  return cmp::make_equal();
}
} // namespace _impl_derive
} // namespace crust


#endif // CRUST_CMP_HPP
