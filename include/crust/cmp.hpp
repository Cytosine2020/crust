#ifndef CRUST_CMP_HPP
#define CRUST_CMP_HPP


#include "cmp_decl.hpp"

#include "crust/enum_decl.hpp"
#include "crust/ops/function.hpp"
#include "crust/option.hpp"
#include "crust/tuple_decl.hpp"
#include "crust/utility.hpp"


namespace crust {
namespace cmp {
struct Less :
    TupleStruct<>,
    AutoImpl<
        Less,
        TupleStruct<>,
        ZeroSizedType,
        cmp::PartialEq,
        cmp::Eq,
        cmp::PartialOrd,
        cmp::Ord> {
  CRUST_USE_BASE_CONSTRUCTORS(Less, TupleStruct<>);
};
struct Equal :
    TupleStruct<>,
    AutoImpl<
        Equal,
        TupleStruct<>,
        ZeroSizedType,
        cmp::PartialEq,
        cmp::Eq,
        cmp::PartialOrd,
        cmp::Ord> {
  CRUST_USE_BASE_CONSTRUCTORS(Equal, TupleStruct<>);
};
struct Greater :
    TupleStruct<>,
    AutoImpl<
        Greater,
        TupleStruct<>,
        ZeroSizedType,
        cmp::PartialEq,
        cmp::Eq,
        cmp::PartialOrd,
        cmp::Ord> {
  CRUST_USE_BASE_CONSTRUCTORS(Greater, TupleStruct<>);
};

class Ordering :
    public Enum<Less, Equal, Greater>,
    public AutoImpl<
        Ordering,
        Enum<Less, Equal, Greater>,
        cmp::PartialEq,
        cmp::Eq>,
    public PartialOrd<Ordering>,
    public Ord<Ordering> {
private:
  crust_cxx17_constexpr i32 to_i32() const {
    return this->template visit<i32>(
        [](const Less &) { return -1; },
        [](const Equal &) { return 0; },
        [](const Greater &) { return 1; });
  }

public:
  CRUST_ENUM_USE_BASE(Ordering, Enum<Less, Equal, Greater>);

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

  /// impl PartialOrd

  crust_cxx17_constexpr Option<Ordering>
  partial_cmp(const Ordering &other) const {
    return make_some(cmp(other));
  }

  /// impl Ord

  crust_cxx17_constexpr Ordering cmp(const Ordering &other) const;
};

always_inline constexpr Ordering make_less() { return Less{}; }

always_inline constexpr Ordering make_equal() { return Equal{}; }

always_inline constexpr Ordering make_greater() { return Greater{}; }

/// because c++ do not support add member function for primitive types,
/// following two functions are used for invoking `partial_cmp' or `cmp' for
/// both struct implemented `PartialOrd' or `Ord' trait and primitive types.
template <class T, class U>
always_inline constexpr Option<Ordering>
operator_partial_cmp(const T &v1, const U &v2) {
  return v1.partial_cmp(v2);
}

template <class T>
always_inline constexpr Ordering operator_cmp(const T &v1, const T &v2) {
  return v1.cmp(v2);
}

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
  always_inline constexpr Ordering operator_cmp(                               \
      const type &v1, const type &v2) {                                        \
    return v1 < v2 ? make_less() : v1 > v2 ? make_greater() : make_equal();    \
  }

_IMPL_PRIMITIVE(_IMPL_OPERATOR_CMP)

#undef _IMPL_OPERATOR_CMP

#define _IMPL_OPERATOR_PARTIAL_CMP(type, ...)                                  \
  always_inline constexpr Option<Ordering> operator_partial_cmp(               \
      const type &v1, const type &v2) {                                        \
    return make_some(operator_cmp(v1, v2));                                    \
  }

_IMPL_PRIMITIVE(_IMPL_OPERATOR_PARTIAL_CMP)

#undef _IMPL_OPERATOR_PARTIAL_CMP

inline crust_cxx17_constexpr Ordering
Ordering::cmp(const Ordering &other) const {
  return operator_cmp(to_i32(), other.to_i32());
}

template <class T>
constexpr T min(T &&v1, T &&v2) {
  crust_static_assert(Derive<T, Ord>::result);
  return move(v1).min(forward<T>(v2));
}

template <class T, class F>
constexpr T
min_by(T &&v1, T &&v2, ops::Fn<F, Ordering(const T &, const T &)> compare) {
  crust_static_assert(Derive<T, Ord>::result);
  return compare(v1, v2) == make_greater() ? forward<T>(v2) : forward<T>(v1);
}

template <class T, class F, class K>
constexpr T min_by_key(T &&v1, T &&v2, ops::Fn<F, K(const T &)> f) {
  crust_static_assert(Derive<T, Ord>::result);
  return min_by(
      forward<T>(v1), forward<T>(v2), ops::bind([&](const T &v1, const T &v2) {
        return operator_cmp(f(v1), f(v2));
      }));
}

template <class T>
constexpr T max(T &&v1, T &&v2) {
  crust_static_assert(Derive<T, Ord>::result);
  return move(v1).max(forward<T>(v2));
}

template <class T, class F>
constexpr T
max_by(T &&v1, T &&v2, ops::Fn<F, Ordering(const T &, const T &)> compare) {
  crust_static_assert(Derive<T, Ord>::result);
  return compare(v1, v2) == make_greater() ? forward<T>(v1) : forward<T>(v2);
}

template <class T, class F, class K>
constexpr T max_by_key(T &&v1, T &&v2, ops::Fn<F, K(const T &)> f) {
  crust_static_assert(Derive<T, Ord>::result);
  return max_by(
      forward<T>(v1), forward<T>(v2), ops::bind([&](const T &v1, const T &v2) {
        return operator_cmp(f(v1), f(v2));
      }));
}

template <class T>
class crust_ebco Reverse :
    public Impl<PartialEq<Reverse<T>>, Derive<T, PartialEq>>,
    public Impl<Eq<Reverse<T>>, Derive<T, Eq>>,
    public Impl<PartialOrd<Reverse<T>>, Derive<T, PartialOrd>>,
    public Impl<Ord<Reverse<T>>, Derive<T, Ord>> {
private:
  T inner;

public:
  CRUST_USE_BASE_CONSTRUCTORS(Reverse, inner);

  /// impl PartialEq

  constexpr bool eq(const Reverse &other) const { return other.inner == inner; }

  /// impl PartialOrd

  constexpr Option<Ordering> partial_cmp(const Reverse &other) const {
    return operator_partial_cmp(other.inner, inner);
  }

  constexpr bool lt(const Reverse &other) const { return other.inner < inner; }

  constexpr bool le(const Reverse &other) const { return other.inner <= inner; }

  constexpr bool gt(const Reverse &other) const { return other.inner > inner; }

  constexpr bool ge(const Reverse &other) const { return other.inner >= inner; }

  /// impl Ord

  constexpr Ordering cmp(const Reverse &other) const {
    return operator_cmp(other.inner, inner);
  }

  crust_cxx14_constexpr Reverse max(const Reverse &&other) {
    return Reverse{inner.min(move(other.inner))};
  }

  crust_cxx14_constexpr Reverse min(const Reverse &&other) {
    return Reverse{inner.max(move(other.inner))};
  }

  crust_cxx14_constexpr Reverse clamp(Reverse &&min, Reverse &&max) {
    return Reverse{inner.clamp(move(max.inner), move(min.inner))};
  }
};
} // namespace cmp

#define _DERIVE_PRIMITIVE(PRIMITIVE, TRAIT, ...)                               \
  struct Derive<PRIMITIVE, TRAIT, ##__VA_ARGS__> : BoolVal<true> {}

_IMPL_PRIMITIVE(_DERIVE_PRIMITIVE, cmp::PartialEq);
_IMPL_PRIMITIVE(_DERIVE_PRIMITIVE, cmp::Eq);
_IMPL_PRIMITIVE(_DERIVE_PRIMITIVE, cmp::PartialOrd);
_IMPL_PRIMITIVE(_DERIVE_PRIMITIVE, cmp::Ord);

#undef _DERIVE_PRIMITIVE
#undef _IMPL_PRIMITIVE

// todo: implement for float point numbers
} // namespace crust


#endif // CRUST_CMP_HPP
