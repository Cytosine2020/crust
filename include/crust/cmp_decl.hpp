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

  constexpr friend bool operator==(const Self &self_, const Rhs &other) {
    return static_cast<const ImplFor<PartialEq<Self>> &>(self_).eq(other);
  }

  constexpr friend bool operator!=(const Self &self_, const Rhs &other) {
    return static_cast<const ImplFor<PartialEq<Self>> &>(self_).ne(other);
  }
};

CRUST_TRAIT(Eq) { CRUST_TRAIT_USE_SELF(Eq, Require<Self, PartialEq>); };

CRUST_TRAIT(PartialOrd, class Rhs = Self) {
  CRUST_TRAIT_USE_SELF(PartialOrd, Require<Self, PartialEq>);

  Option<Ordering> partial_cmp(const Rhs &other) const;

  constexpr bool lt(const Rhs &other) const;

  constexpr bool le(const Rhs &other) const;

  constexpr bool gt(const Rhs &other) const;

  constexpr bool ge(const Rhs &other) const;

  constexpr friend bool operator<(const Self &self_, const Rhs &other) {
    return static_cast<const ImplFor<PartialOrd<Self>> &>(self_).lt(other);
  }

  constexpr friend bool operator<=(const Self &self_, const Rhs &other) {
    return static_cast<const ImplFor<PartialOrd<Self>> &>(self_).le(other);
  }

  constexpr friend bool operator>(const Self &self_, const Rhs &other) {
    return static_cast<const ImplFor<PartialOrd<Self>> &>(self_).gt(other);
  }

  constexpr friend bool operator>=(const Self &self_, const Rhs &other) {
    return static_cast<const ImplFor<PartialOrd<Self>> &>(self_).ge(other);
  }
};

CRUST_TRAIT(Ord) {
  CRUST_TRAIT_USE_SELF(
      Ord,
      Require<Self, PartialEq>,
      Require<Self, Eq>,
      Require<Self, PartialOrd>);

  Ordering cmp(const Self &other) const;

  crust_cxx14_constexpr Self max(Self && other) && {
    return self<PartialOrd<Self>>().gt(other) ?
        move(*static_cast<Self *>(this)) :
        move(other);
  }

  crust_cxx14_constexpr Self min(Self && other) && {
    return self<PartialOrd<Self>>().gt(other) ?
        move(other) :
        move(*static_cast<Self *>(this));
  }

  crust_cxx14_constexpr Self clamp(Self && min, Self && max) && {
    return crust_debug_assert(min <= max),
           self<PartialOrd<Self>>().lt(min) ?
               move(min) :
               (self<PartialOrd<Self>>().gt(max) ?
                    move(max) :
                    move(*static_cast<Self *>(this)));
  }
};
} // namespace cmp

namespace _impl_derive {
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
} // namespace _impl_derive
} // namespace crust


#endif // CRUST_CMP_DECL_HPP
