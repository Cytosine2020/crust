#ifndef CRUST_CMP_DECLARE_HPP
#define CRUST_CMP_DECLARE_HPP


#include "utility.hpp"


namespace crust {
template<class T>
class Option;

namespace cmp {
CRUST_TRAIT(PartialEq, class Rhs = Self)
public:
    /// Used for detecting `PartialEq', part of the workaround below.
    /// Should never be implemented by hand.
    template<class = void>
    void __detect_trait_partial_eq(const Self &) const {}

    bool eq(const Rhs &other) const;

    constexpr bool ne(const Rhs &other) const { return !self().eq(other); }

    constexpr bool operator==(const Rhs &other) const { return self().eq(other); }

    constexpr bool operator!=(const Rhs &other) const { return self().ne(other); }
};

CRUST_TRAIT(Eq)
public:
    /// Used for detecting `Eq', part of the workaround below.
    /// Should never be implemented by hand.
    template<class = void>
    void __detect_trait_eq(const Self &) const {}
};

class Ordering;

CRUST_TRAIT(PartialOrd, class Rhs = Self)
private:
    /// this function will choose `cmp' over `partial_cmp'
    constexpr Option<Ordering> cmp_helper(const Rhs &other) const;

public:
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

CRUST_TRAIT(Ord)
public:
    Ordering cmp(const Self &other) const;

    CRUST_CXX14_CONSTEXPR Self &&max(Self &&other) {
        if (self() > other) {
            return move(self());
        } else {
            return move(other);
        }
    }

    CRUST_CXX14_CONSTEXPR Self &&min(Self &&other) {
        if (self() > other) {
            return move(other);
        } else {
            return move(self());
        }
    }

    CRUST_CXX14_CONSTEXPR Self &&clamp(Self &&min, Self &&max) {
        CRUST_ASSERT(min <= max);
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

namespace __impl_derive_eq {
///
/// Ugly workaround for detecting `PartialEq' and `Eq' for classes inherits `Tuple' and `Enum'.
///
template<typename T, class Rhs>
struct DerivePartialEq {
    template<typename U>
    static u32 check(decltype(static_cast<void (U::*)(const Rhs &) const>(
            &U::template __detect_trait_partial_eq<>)));

    template<typename>
    static void check(...);

    static constexpr bool result = IsSame<decltype(check<T>(0)), u32>::result;
};

template<typename T>
struct DeriveEq {
    template<typename U>
    static u32 check(decltype(static_cast<void (U::*)(const U &) const>(
            &U::template __detect_trait_eq<>)));

    template<typename>
    static void check(...);

    static constexpr bool result = IsSame<decltype(check<T>(0)), u32>::result;
};
}


template<class Self, class Rhs>
struct Derive<Self, cmp::PartialEq<Self, Rhs>> {
static constexpr bool result =__impl_derive_eq::DerivePartialEq<Self, Rhs>::result;
};

template<class Self>
struct Derive<Self, cmp::Eq<Self>> {
static constexpr bool result = __impl_derive_eq::DeriveEq<Self>::result;
};
}


#endif //CRUST_CMP_DECLARE_HPP
