#ifndef CRUST_CMP_DECL_HPP
#define CRUST_CMP_DECL_HPP


#include "crust/utility.hpp"


namespace crust {
namespace option {
template <class T>
class Option;
} // namespace option

using option::Option;

namespace cmp {
class Ordering;

CRUST_TRAIT(PartialEq, class Rhs = Self) {
  CRUST_TRAIT_REQUIRE(PartialEq);

  /// Used for detecting `PartialEq', part of the workaround below.
  /// Should never be implemented by hand.
  template <class = void>
  static void _detect_trait_partial_eq(const Rhs &) {}

  bool eq(const Rhs &other) const;

  constexpr bool ne(const Rhs &other) const { return !self().eq(other); }

  constexpr bool operator==(const Rhs &other) const { return self().eq(other); }

  constexpr bool operator!=(const Rhs &other) const { return self().ne(other); }
};

CRUST_TRAIT(Eq) {
  CRUST_TRAIT_REQUIRE(Eq, Derive<Self, PartialEq>);

  /// Used for detecting `Eq', part of the workaround below.
  /// Should never be implemented by hand.
  template <class = void>
  static void _detect_trait_eq() {}
};

class Ordering;

CRUST_TRAIT(PartialOrd, class Rhs = Self) {
  CRUST_TRAIT_REQUIRE(PartialOrd, Derive<Self, PartialEq>);

  Option<Ordering> partial_cmp(const Rhs &other) const;

  constexpr bool lt(const Rhs &other) const;

  constexpr bool le(const Rhs &other) const;

  constexpr bool gt(const Rhs &other) const;

  constexpr bool ge(const Rhs &other) const;

  constexpr bool operator<(const Rhs &other) const { return self().lt(other); }

  constexpr bool operator<=(const Rhs &other) const { return self().le(other); }

  constexpr bool operator>(const Rhs &other) const { return self().gt(other); }

  constexpr bool operator>=(const Rhs &other) const { return self().ge(other); }
};

CRUST_TRAIT(Ord) {
  CRUST_TRAIT_REQUIRE(
      Ord, Derive<Self, PartialEq>, Derive<Self, Eq>, Derive<Self, PartialOrd>);

  Ordering cmp(const Self &other) const;

  crust_cxx14_constexpr Self max(Self && other) && {
    return self() > other ? move(self()) : move(other);
  }

  crust_cxx14_constexpr Self min(Self && other) && {
    return self() > other ? move(other) : move(self());
  }

  crust_cxx14_constexpr Self clamp(Self && min, Self && max) && {
    crust_debug_assert(min <= max);
    if (self() < min) {
      return move(min);
    } else if (self() > max) {
      return move(max);
    } else {
      return move(self());
    }
  }
};
} // namespace cmp

namespace _impl_derive_eq {
/// Ugly workaround for detecting `PartialEq' and `Eq' for classes inherits
/// `Tuple' and `Enum'.
template <class T, class Rhs>
struct DerivePartialEq {
  template <class U>
  static u32 check(decltype(static_cast<void (*)(const Rhs &)>(
      &U::template _detect_trait_partial_eq<>)));

  template <class>
  static void check(...);
};

template <class T>
struct DeriveEq {
  template <class U>
  static u32
  check(decltype(static_cast<void (*)()>(&U::template _detect_trait_eq<>)));

  template <class>
  static void check(...);
};
} // namespace _impl_derive_eq


template <class Self, class Rhs>
struct Derive<Self, cmp::PartialEq, Rhs> :
    IsSameVal<
        decltype(_impl_derive_eq::DerivePartialEq<Self, Rhs>::template check<
                 Self>(nullptr)),
        u32> {};

template <class Self>
struct Derive<Self, cmp::PartialEq> :
    IsSameVal<
        decltype(_impl_derive_eq::DerivePartialEq<Self, Self>::template check<
                 Self>(nullptr)),
        u32> {};

template <class Self>
struct Derive<Self, cmp::Eq> :
    IsSameVal<
        decltype(_impl_derive_eq::DeriveEq<Self>::template check<Self>(
            nullptr)),
        u32> {};
} // namespace crust


#endif // CRUST_CMP_DECL_HPP
