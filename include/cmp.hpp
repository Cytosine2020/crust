#ifndef CRUST_CMP_HPP
#define CRUST_CMP_HPP


#include "cmp_declare.hpp"

#include "utility.hpp"
#include "option.hpp"
#include "function.hpp"
#include "tuple_declare.hpp"
#include "enum.hpp"


namespace crust {
namespace cmp {
CRUST_ENUM_VARIANT(Less);
CRUST_ENUM_VARIANT(Equal);
CRUST_ENUM_VARIANT(Greater);

class CRUST_EBCO Ordering :
    public Enum<Less, Equal, Greater>,
    public PartialOrd<Ordering>,
    public Ord<Ordering>
{
public:
  CRUST_ENUM_USE_BASE(Ordering, Enum<Less, Equal, Greater>);

private:
  CRUST_CXX14_CONSTEXPR i32 to_i32() const {
    // todo: allow assigning number
    return this->template visit<i32>(
        [](const Less &) { return -1; },
        [](const Equal &) { return 0; },
        [](const Greater &) { return 1; }
    );
  }

public:
  Ordering reverse() const {
    return this->template visit<Ordering>(
        [](const Less &) { return Greater{}; },
        [](const Equal &) { return Equal{}; },
        [](const Greater &) { return Less{}; }
    );
  }

  Ordering then(const Ordering &other) const {
    return this->template visit<Ordering>(
        [](const Less &) { return Less{}; },
        [&](const Equal &) { return other; },
        [](const Greater &) { return Greater{}; }
    );
  }

  template<class F>
  CRUST_CXX14_CONSTEXPR Ordering then_with(Fn<F, Ordering()> f) const {
    return this->template visit<Ordering>(
        [](const Less &) { return Less{}; },
        [&](const Equal &) { return f(); },
        [](const Greater &) { return Greater{}; }
    );
  }

  /// impl PartialOrd

  CRUST_CXX14_CONSTEXPR Option<Ordering>
  partial_cmp(const Ordering &other) const { return make_some(cmp(other)); }

  /// impl Ord

  CRUST_CXX14_CONSTEXPR Ordering cmp(const Ordering &other) const;
};

constexpr Ordering make_less() { return Less{}; }

constexpr Ordering make_equal() { return Equal{}; }

constexpr Ordering make_greater() { return Greater{}; }

/// because c++ do not support add member function for primitive types,
/// following two functions are used for invoking `partial_cmp' or `cmp' for
/// both struct implemented `PartialOrd' or `Ord' trait and primitive types.
template<class T, class U>
constexpr Option<Ordering> operator_partial_cmp(const T &v1, const U &v2) {
  return v1.partial_cmp(v2);
}

template<class T, class U>
constexpr Ordering operator_cmp(const T &v1, const U &v2) {
  return v1.cmp(v2);
}

// todo: refactor
template<class Self, class Rhs>
constexpr bool PartialOrd<Self, Rhs>::lt(const Rhs &other) const {
  return operator_partial_cmp(self(), other).contains(make_less());
}

template<class Self, class Rhs>
constexpr bool PartialOrd<Self, Rhs>::le(const Rhs &other) const {
  return operator_partial_cmp(self(), other) == make_some(make_greater()) ||
       operator_partial_cmp(self(), other) == make_some(make_equal());
}

template<class Self, class Rhs>
constexpr bool PartialOrd<Self, Rhs>::gt(const Rhs &other) const {
  return operator_partial_cmp(self(), other).contains(make_greater());
}

template<class Self, class Rhs>
constexpr bool PartialOrd<Self, Rhs>::ge(const Rhs &other) const {
  return operator_partial_cmp(self(), other) == make_some(make_greater()) ||
       operator_partial_cmp(self(), other) == make_some(make_equal());
}

#define __IMPL_PRIMITIVE(FN, ...) \
  template<> FN(bool, ##__VA_ARGS__); \
  template<> FN(char, ##__VA_ARGS__); \
  template<> FN(u8, ##__VA_ARGS__); \
  template<> FN(i8, ##__VA_ARGS__); \
  template<> FN(u16, ##__VA_ARGS__); \
  template<> FN(i16, ##__VA_ARGS__); \
  template<> FN(u32, ##__VA_ARGS__); \
  template<> FN(i32, ##__VA_ARGS__); \
  template<> FN(u64, ##__VA_ARGS__); \
  template<> FN(i64, ##__VA_ARGS__)

#define __IMPL_OPERATOR_CMP(type, ...) \
inline constexpr Ordering operator_cmp(const type &v1, const type &v2) { \
  return v1 < v2 ? make_less() : v1 > v2 ? make_greater() : make_equal(); \
}

__IMPL_PRIMITIVE(__IMPL_OPERATOR_CMP);

#undef __IMPL_OPERATOR_CMP

#define __IMPL_OPERATOR_PARTIAL_CMP(type, ...) \
inline constexpr Option<Ordering> \
operator_partial_cmp(const type &v1, const type &v2) { \
  return make_some(operator_cmp(v1, v2)); \
}

__IMPL_PRIMITIVE(__IMPL_OPERATOR_PARTIAL_CMP);

#undef __IMPL_OPERATOR_PARTIAL_CMP

inline CRUST_CXX14_CONSTEXPR Ordering
Ordering::cmp(const Ordering &other) const {
  return operator_cmp(to_i32(), other.to_i32());
}

template<class T>
constexpr T min(T &&v1, T &&v2) {
  CRUST_STATIC_ASSERT(Derive<T, Ord>::result);
  return v1.min(forward<T>(v2));
}

template<class T, class F>
constexpr T
min_by(T &&v1, T &&v2, Fn<F, Ordering(const T &, const T &)> compare) {
  CRUST_STATIC_ASSERT(Derive<T, Ord>::result);
  return compare(v1, v2) == make_greater() ? forward<T>(v2) : forward<T>(v1);
}

template<class T, class F, class K>
constexpr T min_by_key(T &&v1, T &&v2, Fn<F, K(const T &)> f) {
  CRUST_STATIC_ASSERT(Derive<T, Ord>::result);
  return min_by(
      forward<T>(v1),
      forward<T>(v2),
      bind([&](const T &v1, const T &v2) {
        return operator_cmp(f(v1), f(v2));
      })
  );
}

template<class T>
constexpr T max(T &&v1, T &&v2) {
  CRUST_STATIC_ASSERT(Derive<T, Ord>::result);
  return v1.max(forward<T>(v2));
}

template<class T, class F>
constexpr T
max_by(T &&v1, T &&v2, Fn<F, Ordering(const T &, const T &)> compare) {
  CRUST_STATIC_ASSERT(Derive<T, Ord>::result);
  return compare(v1, v2) == make_greater() ? forward<T>(v1) : forward<T>(v2);
}

template<class T, class F, class K>
constexpr T max_by_key(T &&v1, T &&v2, Fn<F, K(const T &)> f) {
  CRUST_STATIC_ASSERT(Derive<T, Ord>::result);
  return max_by(
      forward<T>(v1),
      forward<T>(v2),
      bind([&](const T &v1, const T &v2) {
        return operator_cmp(f(v1), f(v2));
      })
  );
}

template<class T>
class Reverse :
    public Impl<PartialEq<Reverse<T>>, Derive<T, PartialEq>::result>,
    public Impl<Eq<Reverse<T>>, Derive<T, Eq>::result>,
    public Impl<PartialOrd<Reverse<T>>, Derive<T, PartialOrd>::result>,
    public Impl<Ord<Reverse<T>>, Derive<T, Ord>::result> {
public:
  T inner;

  /// impl PartialEq

  constexpr bool eq(const Reverse &other) const {
    return inner == other.inner;
  }

  /// impl PartialOrd

  constexpr Option<Ordering> partial_cmp(const Reverse &other) const {
    return operator_partial_cmp(other.inner, this->inner);
  }

  constexpr bool lt(const Reverse &other) const {
    return other.inner < this->inner;
  }

  constexpr bool le(const Reverse &other) const {
    return other.inner <= this->inner;
  }

  constexpr bool gt(const Reverse &other) const {
    return other.inner > this->inner;
  }

  constexpr bool ge(const Reverse &other) const {
    return other.inner >= this->inner;
  }

  /// impl Ord

  constexpr Ordering cmp(const Reverse &other) const {
    return operator_cmp(other.inner, this->inner);
  }
};
}

__IMPL_PRIMITIVE(CRUST_DERIVE_PRIMITIVE, cmp::PartialEq);
__IMPL_PRIMITIVE(CRUST_DERIVE_PRIMITIVE, cmp::Eq);
__IMPL_PRIMITIVE(CRUST_DERIVE_PRIMITIVE, cmp::PartialOrd);
__IMPL_PRIMITIVE(CRUST_DERIVE_PRIMITIVE, cmp::Ord);

#undef __IMPL_PRIMITIVE

// todo: implement for float point numbers
}


#endif //CRUST_CMP_HPP
