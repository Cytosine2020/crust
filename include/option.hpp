#ifndef CRUST_OPTION_HPP
#define CRUST_OPTION_HPP


#include "option_declare.hpp"


namespace crust {
template<class T>
CRUST_ENUM_VARIANT(Some, T);

CRUST_ENUM_VARIANT(None);

template<class T>
constexpr Option<T> make_some(T &&value) {
    return Some<typename RemoveRef<T>::Result>{forward<T>(value)};
}

template<class T>
constexpr Option<T> make_none() { return None{}; }

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

template<class T>
CRUST_CXX14_CONSTEXPR Option<T> Option<T>::take() {
    auto tmp = this->template move_variant<Option<T>>();
    *this = None{};
    return tmp;
}
}


#endif //CRUST_OPTION_HPP
