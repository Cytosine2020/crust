#ifndef CRUST_OPTION_HPP
#define CRUST_OPTION_HPP


#include "option_declare.hpp"

#include "tuple.hpp"


namespace crust {
template<class T>
CRUST_ENUM_FIELD(Some, T);

CRUST_ENUM_FIELD(None);

template<class T>
constexpr Option<T> make_some(T &&value) { return Some<T>{forward<T>(value)}; }

template<class T>
struct AsPtr {
    constexpr Option<const T *> operator()(const Some<T> &value) const {
        return make_some(&value.template get<0>());
    }

    constexpr Option<const T *> operator()(const None &) const { return None{}; }
};

template<class T>
struct AsMutPtr {
    constexpr Option<T *> operator()(Some<T> &value) const {
        return make_some(&value.template get<0>());
    }

    constexpr Option<const T *> operator()(None &) const { return None{}; }
};

template<class T>
template<class F, class Arg, class U>
struct Option<T>::Map {
    Fn<F, U(Arg)> &&f;

    constexpr Option<U> operator()(const Some<T> &value) const {
        return make_some(f(value.template get<0>()));
    }

    constexpr Option<U> operator()(const None &) const { return None{}; }
};
}


#endif //CRUST_OPTION_HPP
