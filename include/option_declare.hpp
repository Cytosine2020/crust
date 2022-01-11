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

    CRUST_CXX14_CONSTEXPR Option<const T *> as_ptr() const {
        return map(make_fn([](const T &value) { return &value; }));
    }

    CRUST_CXX14_CONSTEXPR Option<T *> as_mut_ptr() {
        return map(make_fn([](T &value) { return &value; }));
    }

    CRUST_CXX14_CONSTEXPR T unwrap() {
        return this->template visit<T>(overloaded(
                [](Some<T> &value) { return move(value.template get<0>()); },
                [](None &) { CRUST_PANIC("called `Option::unwrap()` on a `None` value"); }
        ));
    }

    CRUST_CXX14_CONSTEXPR T unwrap_or(T &&d) {
        return this->template visit<T>(overloaded(
                [](Some<T> &value) { return move(value.template get<0>()); },
                [&](None &) { return d; }
        ));
    }

    template<class U, class F>
    CRUST_CXX14_CONSTEXPR Option<U> map(Fn<F, U(const T &)> &&f) const;

    template<class U, class F>
    CRUST_CXX14_CONSTEXPR U map_or(U &&d, Fn<F, U(const T &)> &&f) const {
        return this->template visit<U>(overloaded(
                [&](const Some<T> &value) { return f(value.template get<0>()); },
                [&](const None &) { return move<U>(d); }
        ));
    }

    template<class U, class D, class F>
    CRUST_CXX14_CONSTEXPR U map_or_else(Fn<D, U()> &&d, Fn<F, U(const T &)> &&f) const {
        return this->template visit<U>(overloaded(
                [&](const Some<T> &value) { return f(value.template get<0>()); },
                [&](const None &) { return d(); }
        ));
    }

    CRUST_CXX14_CONSTEXPR Option<T> take();
};
}


#endif //CRUST_OPTION_DECLARE_HPP
