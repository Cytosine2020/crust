#ifndef CRUST_CMP_DECL_HPP
#define CRUST_CMP_DECL_HPP


#include "crust/utility.hpp"


namespace crust {
namespace option {
template <class T>
struct Option;
} // namespace option

using option::Option;

namespace cmp {
struct Ordering;

CRUST_TRAIT(PartialEq, class Rhs = Self) {
  CRUST_TRAIT_USE_SELF(PartialEq);

  bool eq(const Rhs &other) const;

  constexpr bool ne(const Rhs &other) const { return !self().eq(other); }

  constexpr bool operator==(const Rhs &other) const { return self().eq(other); }

  constexpr bool operator!=(const Rhs &other) const { return self().ne(other); }
};

CRUST_TRAIT(Eq) { CRUST_TRAIT_USE_SELF(Eq, Derive<Self, PartialEq>); };

CRUST_TRAIT(PartialOrd, class Rhs = Self) {
  CRUST_TRAIT_USE_SELF(PartialOrd, Derive<Self, PartialEq>);

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
  CRUST_TRAIT_USE_SELF(
      Ord, Derive<Self, PartialEq>, Derive<Self, Eq>, Derive<Self, PartialOrd>);

  Ordering cmp(const Self &other) const;

  crust_cxx14_constexpr Self max(Self && other) && {
    return self() > other ? move(self()) : move(other);
  }

  crust_cxx14_constexpr Self min(Self && other) && {
    return self() > other ? move(other) : move(self());
  }

  crust_cxx14_constexpr Self clamp(Self && min, Self && max) && {
    return crust_debug_assert(min <= max),
           self() < min     ? move(min) :
               self() > max ? move(max) :
                              move(self());
  }
};
} // namespace cmp

namespace _auto_impl {
template <class Self, class Base, usize rev_index = TupleLikeSize<Base>::result>
struct TupleLikePartialEqHelper {
  static constexpr usize index = TupleLikeSize<Base>::result - rev_index;
  using Getter = TupleLikeGetter<Base, index>;
  using Remains = TupleLikePartialEqHelper<Self, Base, rev_index - 1>;

  static constexpr bool eq(const Self &a, const Self &b) {
    return Getter::get(a) == Getter::get(b) && Remains::eq(a, b);
  }

  static constexpr bool ne(const Self &a, const Self &b) {
    return Getter::get(a) != Getter::get(b) || Remains::ne(a, b);
  }
};

template <class Self, class Base>
struct TupleLikePartialEqHelper<Self, Base, 0> {
  static constexpr bool eq(const Self &, const Self &) { return true; }

  static constexpr bool ne(const Self &, const Self &) { return false; }
};

template <class Self, class Base, usize rev_index = TupleLikeSize<Base>::result>
struct TupleLikePartialOrdHelper {
  static constexpr usize index = TupleLikeSize<Base>::result - rev_index;
  using Getter = TupleLikeGetter<Base, index>;
  using Remains = TupleLikePartialOrdHelper<Self, Base, rev_index - 1>;

  static constexpr Option<cmp::Ordering>
  partial_cmp(const Self &a, const Self &b);

  static constexpr bool lt(const Self &a, const Self &b) {
    return Getter::get(a) != Getter::get(b) ? Getter::get(a) < Getter::get(b) :
                                              Remains::lt(a, b);
  }

  static constexpr bool le(const Self &a, const Self &b) {
    return Getter::get(a) != Getter::get(b) ? Getter::get(a) <= Getter::get(b) :
                                              Remains::le(a, b);
  }

  static constexpr bool gt(const Self &a, const Self &b) {
    return Getter::get(a) != Getter::get(b) ? Getter::get(a) > Getter::get(b) :
                                              Remains::gt(a, b);
  }

  static constexpr bool ge(const Self &a, const Self &b) {
    return Getter::get(a) != Getter::get(b) ? Getter::get(a) >= Getter::get(b) :
                                              Remains::ge(a, b);
  }
};

template <class Self, class Base>
struct TupleLikePartialOrdHelper<Self, Base, 0> {
  static constexpr Option<cmp::Ordering>
  partial_cmp(const Self &, const Self &);

  static constexpr bool lt(const Self &, const Self &) { return false; }

  static constexpr bool le(const Self &, const Self &) { return true; }

  static constexpr bool gt(const Self &, const Self &) { return false; }

  static constexpr bool ge(const Self &, const Self &) { return true; }
};

template <class Self, class Base, usize rev_index = TupleLikeSize<Base>::result>
struct TupleLikeOrdHelper {
  static constexpr usize index = TupleLikeSize<Base>::result - rev_index;
  using Getter = TupleLikeGetter<Base, index>;
  using Remains = TupleLikeOrdHelper<Self, Base, rev_index - 1>;

  static constexpr cmp::Ordering cmp(const Self &a, const Self &b);
};

template <class Self, class Base>
struct TupleLikeOrdHelper<Self, Base, 0> {
  static constexpr cmp::Ordering cmp(const Self &, const Self &);
};

template <class Self>
struct AutoImpl<Self, MonoStateType, cmp::PartialEq> : cmp::PartialEq<Self> {
  constexpr bool eq(const Self &) const { return true; }

  constexpr bool ne(const Self &) const { return false; }
};

template <class Self>
struct AutoImpl<Self, MonoStateType, cmp::Eq, void> : cmp::Eq<Self> {};

template <class Self>
struct AutoImpl<Self, MonoStateType, cmp::PartialOrd, void> :
    cmp::PartialOrd<Self> {
  constexpr Option<cmp::Ordering> partial_cmp(const Self &) const;

  constexpr bool lt(const Self &) const { return false; }

  constexpr bool le(const Self &) const { return true; }

  constexpr bool gt(const Self &) const { return false; }

  constexpr bool ge(const Self &) const { return true; }
};

template <class Self>
struct AutoImpl<Self, MonoStateType, cmp::Ord, void> : cmp::Ord<Self> {
  constexpr cmp::Ordering cmp(const Self &) const;
};
} // namespace _auto_impl
} // namespace crust


#endif // CRUST_CMP_DECL_HPP
