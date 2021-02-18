#ifndef CRUST_OPTION_HPP
#define CRUST_OPTION_HPP


#include "option_declare.hpp"

#include "tuple.hpp"


namespace crust {
template<class T>
CRUST_ENUM_FIELD(Some, T);

CRUST_ENUM_FIELD(None);

template<class T>
Option<T> make_some(T &&value) { return Some<T>{forward<T>(value)}; }

template<class T>
struct AsPtr {
    Option<const T *> operator()(const Some<T> &value) {
        return make_some(&value.template get<0>());
    }

    Option<const T *> operator()(const None &) { return None{}; }
};

template<class T>
struct AsMutPtr {
    Option<T *> operator()(Some<T> &value) { return make_some(&value.template get<0>()); }

    Option<const T *> operator()(None &) { return None{}; }
};

template<class T>
template<class F, class Arg, class U>
struct Option<T>::Map {
    const Fn<F, U(Arg)> &f;

    Option<U> operator()(const Some<T> &value) { return make_some(f(value.template get<0>())); }

    Option<U> operator()(const None &) { return None{}; }
};
}


#endif //CRUST_OPTION_HPP
