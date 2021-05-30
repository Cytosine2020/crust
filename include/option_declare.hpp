#ifndef CRUST_OPTION_DECLARE_HPP
#define CRUST_OPTION_DECLARE_HPP


#include "utility.hpp"
#include "function.hpp"
#include "enum_declare.hpp"
#include "cmp_declare.hpp"
#include "tuple_declare.hpp"


namespace crust {
template<class T>
struct Some;

struct None;

template<class T>
class Option : public Enum<Some<T>, None> {
public:
    CRUST_ENUM_USE_BASE(Option, Enum<Some<T>, None>);

    constexpr bool is_some() const { return this->template is_variant<Some<T>>(); }

    constexpr bool is_none() const { return this->template is_variant<None>(); }

    CRUST_CXX14_CONSTEXPR bool contains(const T &other) const {
        return this->template eq_variant<T>(other);
    }

private:
    struct AsPtr;

public:
    CRUST_CXX14_CONSTEXPR Option<const T *> as_ptr() const {
        return this->template visit<Option<const T *>>(AsPtr{});
    }

private:
    struct AsMutPtr;

public:
    CRUST_CXX14_CONSTEXPR Option<T *> as_mut_ptr() {
        return this->template visit<Option<T *>>(AsMutPtr{});
    }

private:
    struct Unwrap {
        constexpr T &&operator()(Some<T> &value) const { return move(value); }

        CRUST_CXX14_CONSTEXPR T &&operator()(None &) const {
            CRUST_PANIC("called `Option::unwrap()` on a `None` value");
        }
    };

public:
    CRUST_CXX14_CONSTEXPR T &&unwrap() { return this->template visit<T &&>(Unwrap{}); }

private:
    struct UnwrapOr {
        T &&d;

        CRUST_CXX14_CONSTEXPR T &&operator()(Some<T> &value) { return move(value); }

        CRUST_CXX14_CONSTEXPR T &&operator()(None &) { return move(d); }
    };

public:
    CRUST_CXX14_CONSTEXPR T &&unwrap_or(T &&d) {
        return this->template visit<T &&>(UnwrapOr{d});
    }

private:
    template<class F, class Arg, class U>
    struct Map;

public:
    template<class F, class Arg, class U>
    CRUST_CXX14_CONSTEXPR Option<U> map(Fn<F, U(Arg)> &&f) const {
        return this->template visit<Option<U>>(Map<F, Arg, U>{move(f)});
    }

private:
    template<class F, class Arg, class U>
    struct MapOr {
        U &&d;
        Fn<F, U(Arg)> &&f;

        CRUST_CXX14_CONSTEXPR U operator()(const Some<T> &value) {
            return f(value.template get<0>());
        }

        CRUST_CXX14_CONSTEXPR U operator()(const None &) { return forward<U>(d); }
    };

public:
    template<class F, class Arg, class U>
    CRUST_CXX14_CONSTEXPR U map_or(U &&d, Fn<F, U(Arg)> &&f) const {
        return this->template visit<U>(MapOr<F, Arg, U>{d, move(f)});
    }

    CRUST_CXX14_CONSTEXPR Option<T> take();
};
}


#endif //CRUST_OPTION_DECLARE_HPP
