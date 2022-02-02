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
template<class U, class F>
CRUST_CXX14_CONSTEXPR Option<U>
Option<T>::map(Fn<F, U(const T &)> f) const {
  return this->template visit<Option<U>>(
      [&](const Some<T> &value) {
        return make_some(f(value.template get<0>()));
      },
      [](const None &) { return make_none<U>(); }
  );
}

template<class T>
CRUST_CXX14_CONSTEXPR Option<T> Option<T>::take() {
  auto tmp = this->template move_variant<Option<T>>();
  *this = None{};
  return tmp;
}
}


#endif //CRUST_OPTION_HPP
