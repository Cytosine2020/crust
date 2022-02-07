#ifndef _CRUST_INCLUDE_CMP_DECL_HPP
#define _CRUST_INCLUDE_CMP_DECL_HPP


#include "utility.hpp"


namespace crust {
namespace option {
template<class T>
class Option;
}

using option::Option;

namespace cmp {
class Ordering;

CRUST_TRAIT(PartialEq, class Rhs = Self) {
  CRUST_TRAIT_REQUIRE(PartialEq);

  /// Used for detecting `PartialEq', part of the workaround below.
  /// Should never be implemented by hand.
  template<class = void>
  static void _detect_trait_partial_eq(const Rhs &) {}

  bool eq(const Rhs &other) const;

  constexpr bool ne(const Rhs &other) const {
    return !self().eq(other);
  }

  constexpr bool operator==(const Rhs &other) const {
    return self().eq(other);
  }

  constexpr bool operator!=(const Rhs &other) const {
    return self().ne(other);
  }
};

CRUST_TRAIT(Eq) {
  CRUST_TRAIT_REQUIRE(Eq, Derive<Self, PartialEq>);

  /// Used for detecting `Eq', part of the workaround below.
  /// Should never be implemented by hand.
  template<class = void>
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

  constexpr bool operator<(const Rhs &other) const {
    return self().lt(other);
  }

  constexpr bool operator<=(const Rhs &other) const {
    return self().le(other);
  }

  constexpr bool operator>(const Rhs &other) const {
    return self().gt(other);
  }

  constexpr bool operator>=(const Rhs &other) const {
    return self().ge(other);
  }
};

CRUST_TRAIT(Ord) {
  CRUST_TRAIT_REQUIRE(Ord,
    Derive<Self, PartialEq>,
    Derive<Self, Eq>,
    Derive<Self, PartialOrd>
  );

  Ordering cmp(const Self &other) const;

  crust_cxx14_constexpr Self max(Self &&other) {
    if (self() > other) {
      return move(self());
    } else {
      return move(other);
    }
  }

  crust_cxx14_constexpr Self min(Self &&other) {
    if (self() > other) {
      return move(other);
    } else {
      return move(self());
    }
  }

  crust_cxx14_constexpr Self clamp(Self &&min, Self &&max) {
    crust_assert(min <= max);
    if (self() < min) {
      return move(min);
    } else if (self() > max) {
      return move(max);
    } else {
      return move(self());
    }
  }
};
}

namespace _impl_derive_eq {
/// Ugly workaround for detecting `PartialEq' and `Eq' for classes inherits
/// `Tuple' and `Enum'.
template<class T, class Rhs>
struct DerivePartialEq {
  template<class U>
  static u32 check(decltype(static_cast<void (*)(const Rhs &)>(
      &U::template _detect_trait_partial_eq<>
  )));

  template<class>
  static void check(...);

  static constexpr bool result =
      IsSame<decltype(check<T>(nullptr)), u32>::result;
};

template<class T>
struct DeriveEq {
  template<class U>
  static u32 check(decltype(static_cast<void (*)()>(
      &U::template _detect_trait_eq<>
  )));

  template<class>
  static void check(...);

  static constexpr bool result =
      IsSame<decltype(check<T>(nullptr)), u32>::result;
};
}


template<class Self, class Rhs>
struct Derive<Self, cmp::PartialEq, Rhs> :
    public _impl_derive_eq::DerivePartialEq<Self, Rhs>
{};

template<class Self>
struct Derive<Self, cmp::PartialEq> :
    public _impl_derive_eq::DerivePartialEq<Self, Self>
{};

template<class Self>
struct Derive<Self, cmp::Eq> : public _impl_derive_eq::DeriveEq<Self> {};
}


#endif //_CRUST_INCLUDE_CMP_DECL_HPP
