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
        public PartialOrd<Ordering>, public Ord<Ordering> {
public:
    CRUST_ENUM_USE_BASE(Ordering, Enum<Less, Equal, Greater>);

private:
    struct __Reverse {
        constexpr Ordering operator()(const Less &) const { return Greater{}; }

        constexpr Ordering operator()(const Equal &) const { return Equal{}; }

        constexpr Ordering operator()(const Greater &) const { return Less{}; }
    };

public:
    Ordering reverse() const { return this->template visit<Ordering>(__Reverse{}); }

private:
    struct __Then {
        const Ordering &other;

        constexpr Ordering operator()(const Equal &) const { return other; }

        template<class T>
        constexpr Ordering operator()(const T &) const { return T{}; }
    };

public:
    Ordering then(const Ordering &other) const {
        return this->template visit<Ordering>(__Then{other});
    }

private:
    template<class F>
    struct __ThenWith {
        Fn<F, Ordering()> &&f;

        CRUST_CXX14_CONSTEXPR Ordering operator()(const Equal &) { return f(); }

        template<class T>
        CRUST_CXX14_CONSTEXPR Ordering operator()(const T &) { return T{}; }
    };

public:
    template<class F>
    CRUST_CXX14_CONSTEXPR Ordering then_with(Fn<F, Ordering()> &&f) const {
        return this->template visit<Ordering>(__ThenWith<F>{move(f)});
    }

private:
    struct __ToI32 {
        constexpr i32 operator()(const Less &) const { return -1; }

        constexpr i32 operator()(const Equal &) const { return 0; }

        constexpr i32 operator()(const Greater &) const { return 1; }
    };

    CRUST_CXX14_CONSTEXPR i32 to_i32() const { return this->template visit<i32>(__ToI32{}); }

public:
    /// impl PartialOrd

    CRUST_CXX14_CONSTEXPR Option<Ordering> partial_cmp(const Ordering &other) const;

    /// impl Ord

    CRUST_CXX14_CONSTEXPR Ordering cmp(const Ordering &other) const;
};

constexpr Ordering make_less() { return Less{}; }

constexpr Ordering make_equal() { return Equal{}; }

constexpr Ordering make_greater() { return Greater{}; }

namespace __impl_cmp {
template<class T, class U, bool = Derive<T, Ord>::result>
struct CmpHelper {
    constexpr static Option<Ordering> inner(const T &v1, const U &v2) {
        return operator_partial_cmp(v1, v2);
    }
};

template<class T>
struct CmpHelper<T, T, true> {
    constexpr static Option<Ordering> inner(const T &v1, const T &v2) {
        return make_some(operator_cmp(v1, v2));
    }
};
}

template<class Self, class Rhs>
constexpr Option<Ordering> PartialOrd<Self, Rhs>::cmp_helper(const Rhs &other) const {
    return __impl_cmp::CmpHelper<Self, Rhs>::inner(self(), other);
}

template<class Self, class Rhs>
constexpr bool PartialOrd<Self, Rhs>::lt(const Rhs &other) const {
    return self().cmp_helper(other) == make_some(make_less());
}

template<class Self, class Rhs>
constexpr bool PartialOrd<Self, Rhs>::le(const Rhs &other) const {
    return self().cmp_helper(other) == make_some(make_less()) ||
           self().cmp_helper(other) == make_some(make_equal());
}

template<class Self, class Rhs>
constexpr bool PartialOrd<Self, Rhs>::gt(const Rhs &other) const {
    return self().cmp_helper(other) == make_some(make_greater());
}

template<class Self, class Rhs>
constexpr bool PartialOrd<Self, Rhs>::ge(const Rhs &other) const {
    return self().cmp_helper(other) == make_some(make_greater()) ||
           self().cmp_helper(other) == make_some(make_equal());
}


/// because c++ do not support add member function for primitive types (also <=> is only
/// available for c++20), following two functions are used for invoking `partial_cmp' or `cmp'
/// for both struct implemented `PartialOrd' or `Ord' trait and primitive types.
template<class T, class U>
constexpr Option<Ordering> operator_partial_cmp(const T &v1, const U &v2) {
    return v1.partial_cmp(v2);
}

template<class T, class U>
constexpr Ordering operator_cmp(const T &v1, const U &v2) {
    return v1.cmp(v2);
}

#define IMPL_OPERATOR_CMP(type) \
template<> \
inline constexpr Ordering operator_cmp(const type &v1, const type &v2) { \
    return v1 < v2 ? make_less() : v1 > v2 ? make_greater() : make_equal(); \
}\
template<> \
inline constexpr Option<Ordering> operator_partial_cmp(const type &v1, const type &v2) { \
    return make_some(operator_cmp(v1, v2)); \
}

IMPL_OPERATOR_CMP(char);

IMPL_OPERATOR_CMP(u8);

IMPL_OPERATOR_CMP(i8);

IMPL_OPERATOR_CMP(u16);

IMPL_OPERATOR_CMP(i16);

IMPL_OPERATOR_CMP(u32);

IMPL_OPERATOR_CMP(i32);

IMPL_OPERATOR_CMP(u64);

IMPL_OPERATOR_CMP(i64);

#undef IMPL_OPERATOR_CMP

inline CRUST_CXX14_CONSTEXPR Option<Ordering> Ordering::partial_cmp(const Ordering &other) const {
    return operator_partial_cmp(this->to_i32(), other.to_i32());
}

inline CRUST_CXX14_CONSTEXPR Ordering Ordering::cmp(const Ordering &other) const {
    return operator_cmp(this->to_i32(), other.to_i32());
}

template<class T>
constexpr T min(T &&v1, T &&v2) {
    CRUST_STATIC_ASSERT(Derive<T, Ord>::result);
    return v1.min(std::forward<T>(v2));
}

template<class T, class F>
constexpr T min_by(T &&v1, T &&v2, Fn<F, Ordering(const T &, const T &)> &&compare) {
    CRUST_STATIC_ASSERT(Derive<T, Ord>::result);
    return compare(v1, v2) == make_greater() ? forward<T>(v2) : forward<T>(v1);
}

template<class T, class F, class K>
constexpr T min_by_key(T &&v1, T &&v2, Fn<F, K(const T &)> &&f) {
    CRUST_STATIC_ASSERT(Derive<T, Ord>::result);
    return min_by(forward<T>(v1), forward<T>(v2), make_fn([&](const T &v1, const T &v2) {
        return operator_cmp(f(v1), f(v2));
    }));
}

template<class T>
constexpr T &&max(T &&v1, T &&v2) {
    CRUST_STATIC_ASSERT(Derive<T, Ord>::result);
    return v1.max(std::forward<T>(v2));
}

template<class T, class F>
constexpr T &&max_by(T &&v1, T &&v2, Fn<F, Ordering(const T &, const T &)> &&compare) {
    CRUST_STATIC_ASSERT(Derive<T, Ord>::result);
    return compare(v1, v2) == make_greater() ? forward<T>(v1) : forward<T>(v2);
}

template<class T, class F, class K>
constexpr T &&max_by_key(T &&v1, T &&v2, Fn<F, K(const T &)> &&f) {
    CRUST_STATIC_ASSERT(Derive<T, Ord>::result);
    return max_by(forward<T>(v1), forward<T>(v2), make_fn([&](const T &v1, const T &v2) {
        return operator_cmp(f(v1), f(v2));
    }));
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

    constexpr bool eq(const Reverse &other) const { return inner == other.inner; }

    /// impl PartialOrd

    constexpr Option<Ordering> partial_cmp(const Reverse &other) const {
        return operator_partial_cmp(other.inner, this->inner);
    }

    constexpr bool lt(const Reverse &other) const { return other.inner < this->inner; }

    constexpr bool le(const Reverse &other) const { return other.inner <= this->inner; }

    constexpr bool gt(const Reverse &other) const { return other.inner > this->inner; }

    constexpr bool ge(const Reverse &other) const { return other.inner >= this->inner; }

    /// impl Ord

    constexpr Ordering cmp(const Reverse &other) const {
        return operator_cmp(other.inner, this->inner);
    }
};
}

#define IMPL_PRIMITIVE(Trait) \
    CRUST_DERIVE_PRIMITIVE(char, Trait); \
    CRUST_DERIVE_PRIMITIVE(u8, Trait); \
    CRUST_DERIVE_PRIMITIVE(i8, Trait); \
    CRUST_DERIVE_PRIMITIVE(u16, Trait); \
    CRUST_DERIVE_PRIMITIVE(i16, Trait); \
    CRUST_DERIVE_PRIMITIVE(u32, Trait); \
    CRUST_DERIVE_PRIMITIVE(i32, Trait); \
    CRUST_DERIVE_PRIMITIVE(u64, Trait); \
    CRUST_DERIVE_PRIMITIVE(i64, Trait)

CRUST_DERIVE_PRIMITIVE(bool, cmp::PartialEq);

IMPL_PRIMITIVE(cmp::PartialEq);

CRUST_DERIVE_PRIMITIVE(bool, cmp::Eq);

IMPL_PRIMITIVE(cmp::Eq);

IMPL_PRIMITIVE(cmp::PartialOrd);

IMPL_PRIMITIVE(cmp::Ord);

#undef IMPL_PRIMATIVE

// todo: implement for float point numbers
}


#endif //CRUST_CMP_HPP
