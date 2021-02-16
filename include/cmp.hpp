#ifndef CRUST_CMP_HPP
#define CRUST_CMP_HPP


#include "utility.hpp"
#include "function.hpp"
#include "option.hpp"


namespace crust {
CRUST_TRAIT(PartialEq, class Rhs = Self)
public:
    bool eq(const Rhs &other) const;

    bool ne(const Rhs &other) const { return !self().eq(other); }

    bool operator==(const Rhs &other) const { return self().eq(other); }

    bool operator!=(const Rhs &other) const { return self().ne(other); }
};

CRUST_TRAIT(Eq, class Rhs = Self)
//    CRUST_REQUIRE(PartialEq);
};

class Ordering;

CRUST_TRAIT(PartialOrd, class Rhs = Self)
private:
    /// this function will choose `cmp' over `partial_cmp'
    Option<Ordering> cmp_helper(const Rhs &other) const;

public:
    Option<Ordering> partial_cmp(const Rhs &other) const;

    bool lt(const Rhs &other) const;

    bool le(const Rhs &other) const;

    bool gt(const Rhs &other) const;

    bool ge(const Rhs &other) const;

    bool operator<(const Rhs &other) const { return self().lt(other); }

    bool operator<=(const Rhs &other) const { return self().le(other); }

    bool operator>(const Rhs &other) const { return self().gt(other); }

    bool operator>=(const Rhs &other) const { return self().ge(other); }
};

CRUST_TRAIT(Ord)
public:
    Ordering cmp(const Self &other) const;

    Self max(const Self &other) const {
        if (self() > other) {
            return self();
        } else {
            return other;
        }
    }

    Self min(const Self &other) const {
        if (self() > other) {
            return other;
        } else {
            return self();
        }
    }

    Self clamp(const Self &min, const Self &max) const {
        CRUST_ASSERT(min <= max);
        if (self() < min) {
            return min;
        } else if (self() > max) {
            return max;
        } else {
            return self();
        }
    }
};


class Ordering :
        public PartialEq<Ordering>, public Eq<Ordering>,
        public PartialOrd<Ordering>, public Ord<Ordering> {
private:
    enum Enum : i8 {
        Less = -1,
        Equal = 0,
        Greater = 1,
    } value;

    constexpr explicit Ordering(Enum value) : value{value} {}

public:
    static constexpr Ordering less() { return Ordering{Less}; }

    static constexpr Ordering equal() { return Ordering{Equal}; }

    static constexpr Ordering greater() { return Ordering{Greater}; }

    Ordering reverse() const {
        switch (value) {
            CRUST_DEFAULT_UNREACHABLE;
            case Less:
                return greater();
            case Equal:
                return equal();
            case Greater:
                return less();
        }
    }

    Ordering then(Ordering other) const {
        switch (value) {
            CRUST_DEFAULT_UNREACHABLE;
            case Equal:
                return other;
            case Less:
            case Greater:
                return *this;
        }
    }

    template<class F>
    Ordering then_with(const Fn<F, Ordering()> &f) const {
        switch (value) {
            CRUST_DEFAULT_UNREACHABLE;
            case Equal:
                return f();
            case Less:
            case Greater:
                return *this;
        }
    }

    /// impl PartialEq

    bool eq(const Ordering &other) const { return value == other.value; }

    /// impl PartialOrd

    Option<Ordering> partial_cmp(const Ordering &other) const;

    /// impl Ord

    Ordering cmp(const Ordering &other) const;
};

namespace __impl {
template<class T, class U, bool=CRUST_DERIVE(T, Ord)>
struct CmpHelper {
    static Option<Ordering> inner(const T &v1, const U &v2) {
        return operator_partial_cmp(v1, v2);
    }
};

template<class T>
struct CmpHelper<T, T, true> {
    static Option<Ordering> inner(const T &v1, const T &v2) {
        return make_some(operator_cmp(v1, v2));
    }
};
}

template<class Self, class Rhs>
Option<Ordering> PartialOrd<Self, Rhs>::cmp_helper(const Rhs &other) const {
    return __impl::CmpHelper<Self, Rhs>::inner(self(), other);
}

template<class Self, class Rhs>
bool PartialOrd<Self, Rhs>::lt(const Rhs &other) const {
    return self().cmp_helper(other) == make_some(Ordering::less());
}

template<class Self, class Rhs>
bool PartialOrd<Self, Rhs>::le(const Rhs &other) const {
    return self().cmp_helper(other) == make_some(Ordering::less()) ||
           self().cmp_helper(other) == make_some(Ordering::equal());
}

template<class Self, class Rhs>
bool PartialOrd<Self, Rhs>::gt(const Rhs &other) const {
    return self().cmp_helper(other) == make_some(Ordering::greater());
}

template<class Self, class Rhs>
bool PartialOrd<Self, Rhs>::ge(const Rhs &other) const {
    return self().cmp_helper(other) == make_some(Ordering::greater()) ||
           self().cmp_helper(other) == make_some(Ordering::equal());
}

template<class T>
T min(const Ord<T> &v1, const Ord<T> &v2) { return v1.min(v2); }

template<class T, class F>
T min_by(const Ord<T> &v1, const Ord<T> &v2,
         const Fn<F, Ordering(const Ord<T> &, const Ord<T> &)> &compare) {
    if (compare(v1, v2) == Ordering::greater()) {
        return v2.self();
    } else {
        return v1.self();
    }
}

template<class T, class F, class K>
T min_by_key(const Ord<T> &v1, const Ord<T> &v2, const Fn<F, K(const Ord<T> &)> &f) {
    return min_by(v1, v2, make_fn([&]() { return f(v1).cmp(f(v2)); }));
}

template<class T>
T max(const Ord<T> &v1, const Ord<T> &v2) { return v1.max(v2); }

template<class T, class F>
T max_by(const Ord<T> &v1, const Ord<T> &v2,
         const Fn<F, Ordering(const Ord<T> &, const Ord<T> &)> &compare) {
    if (compare(v1, v2) == Ordering::greater()) {
        return v1.self();
    } else {
        return v2.self();
    }
}

template<class T, class F, class K>
T max_by_key(const Ord<T> &v1, const Ord<T> &v2, const Fn<F, K(const Ord<T> &)> &f) {
    return max_by(v1, v2, make_fn([&]() { return f(v1).cmp(f(v2)); }));
}


/// because c++ do not support add member function for primitive types (also <=> is only
/// available for c++20), following two functions are for invoking `partial_cmp' or `cmp'
/// for both struct implemented `PartialOrd' or `Ord' trait and primitive types.
template<class T, class U>
Option<Ordering> operator_partial_cmp(const T &v1, const U &v2) { return v1.partial_cmp(v2); }

template<class T, class U>
Ordering operator_cmp(const T &v1, const U &v2) { return v1.cmp(v2); }

#define IMPL_OPERATOR_CMP(type) \
template<> inline Ordering operator_cmp(const type &v1, const type &v2) { \
    if (v1 < v2) { return Ordering::less(); } \
    else if (v1 > v2) { return Ordering::greater(); } \
    else { return Ordering::equal(); } \
}\
template<> inline Option<Ordering> operator_partial_cmp(const type &v1, const type &v2) { \
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

Option<Ordering> Ordering::partial_cmp(const Ordering &other) const {
    return operator_partial_cmp(static_cast<i8>(this->value), static_cast<i8>(other.value));
}

Ordering Ordering::cmp(const Ordering &other) const {
    return operator_cmp(static_cast<i8>(this->value), static_cast<i8>(other.value));
}

template<class T>
class Reverse :
        public Impl<PartialEq<Reverse<T>>, CRUST_DERIVE(T, PartialEq)>,
        public Impl<Eq<Reverse<T>>, CRUST_DERIVE(T, Eq)>,
        public Impl<PartialOrd<Reverse<T>>, CRUST_DERIVE(T, PartialOrd)>,
        public Impl<Ord<Reverse<T>>, CRUST_DERIVE(T, Ord)> {
public:
    T inner;

    /// impl PartialEq

    bool eq(const Reverse &other) const { return inner == other.inner; }

    /// impl PartialOrd

    Option<Ordering> partial_cmp(const Reverse &other) const {
        return operator_partial_cmp(other.inner, this->inner);
    }

    bool lt(const Reverse &other) const { return other.inner < this->inner; }

    bool le(const Reverse &other) const { return other.inner <= this->inner; }

    bool gt(const Reverse &other) const { return other.inner > this->inner; }

    bool ge(const Reverse &other) const { return other.inner >= this->inner; }

    /// impl Ord

    Ordering cmp(const Reverse &other) const { return operator_cmp(other.inner, this->inner); }
};

#define IMPL_PRIMITIVE(Trait) \
    CRUST_DERIVE_PRIMITIVE(char, Trait); \
    CRUST_DERIVE_PRIMITIVE(u8, Trait); \
    CRUST_DERIVE_PRIMITIVE(i8, Trait); \
    CRUST_DERIVE_PRIMITIVE(u16, Trait); \
    CRUST_DERIVE_PRIMITIVE(i16, Trait); \
    CRUST_DERIVE_PRIMITIVE(u32, Trait); \
    CRUST_DERIVE_PRIMITIVE(i32, Trait); \
    CRUST_DERIVE_PRIMITIVE(u64, Trait); \
    CRUST_DERIVE_PRIMITIVE(i64, Trait); \
    CRUST_DERIVE_PRIMITIVE(u128, Trait); \
    CRUST_DERIVE_PRIMITIVE(i128, Trait)

CRUST_DERIVE_PRIMITIVE(bool, PartialEq);

IMPL_PRIMITIVE(PartialEq);

CRUST_DERIVE_PRIMITIVE(bool, Eq);

IMPL_PRIMITIVE(Eq);

IMPL_PRIMITIVE(PartialOrd);

IMPL_PRIMITIVE(Ord);

#undef IMPL_PRIMATIVE

// todo: implement for float point numbers
}


#endif //CRUST_CMP_HPP
