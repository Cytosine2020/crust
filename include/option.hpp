#ifndef CRUST_OPTION_HPP
#define CRUST_OPTION_HPP


#include "utility.hpp"
#include "function.hpp"
#include "enum.hpp"


namespace crust {
CRUST_TRAIT_DECLARE(PartialEq, class Rhs);

CRUST_TRAIT_DECLARE(Eq, class Rhs);

template<class T>
class Option :
        public Impl<PartialEq<Option<T>, Option<T>>, CRUST_DERIVE(T, PartialEq, T)>,
        public Impl<Eq<Option<T>, Option<T>>, CRUST_DERIVE(T, Eq, T)> {
private:
    struct Monostate : public PartialEq<Monostate, Monostate>, public Eq<Monostate, Monostate> {
        /// impl PartialEq
        bool eq(const Monostate &) const { return true; }
    };

    Enum<T, Monostate> inner;

    Option() noexcept: inner{Monostate{}} {}

    explicit Option(T &&value) : inner{move(value)} {}

public:
    static Option some(T &&inner) { return Option{move(inner)}; }

    static Option none() { return Option{}; }

    template<class R, class V>
    R visit(V &&visitor) const { return inner.template visit<R, V>(visitor); }

    template<class R, class V>
    R visit(V &&visitor) { return inner.template visit<R, V>(visitor); }

private:
    struct IsSome {
        bool operator()(const T &) { return true; }

        bool operator()(const Monostate &) { return false; }
    };

public:
    bool is_some() const { return inner.template visit<bool>(IsSome{}); }

private:
    struct IsNone {
        bool operator()(const T &) { return false; }

        bool operator()(const Monostate &) { return true; }
    };

public:
    bool is_none() const { return inner.template visit<bool>(IsNone{}); }

private:
    template<class F, class Arg, class U>
    struct Map {
        const Fn<F, U(Arg)> &f;

        Option<U> operator()(const T &value) { return Option<U>{f(value)}; }

        Option<U> operator()(const Monostate &) { return Option<U>{}; }
    };

public:
    template<class F, class Arg, class U>
    Option<U> map(const Fn<F, U(Arg)> &f) const {
        return inner.template visit<Option<U>>(Map<F, Arg, U>{f});
    }

private:
    template<class F, class Arg, class U>
    struct MapOr {
        U &&d;
        const Fn<F, U(Arg)> &f;

        Option<U> operator()(const T &value) { return f(value); }

        Option<U> operator()(const Monostate &) { return forward<U>(d); }
    };

public:
    template<class F, class Arg, class U>
    U map_or(U &&d, const Fn<F, U(Arg)> &f) const {
        return inner.template visit<Option<U>>(MapOr<F, Arg, U>{d, f});
    }

    /// impl PartialEq

    bool eq(const Option &other) const { return this->inner == other.inner; }

    bool ne(const Option &other) const { return this->inner != other.inner; }
};

template<class T>
Option<T> make_some(T &&value) { return Option<T>::some(forward<T>(value)); }

template<class T>
Option<T> make_none() { return Option<T>::none(); }

}


#endif //CRUST_OPTION_HPP
