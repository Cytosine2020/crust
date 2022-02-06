#ifndef _CRUST_INCLUDE_OPTION_HPP
#define _CRUST_INCLUDE_OPTION_HPP


#include "option_decl.hpp"


namespace crust {
namespace option {
template<class T>
CRUST_ENUM_VARIANT(Some, T);

CRUST_ENUM_VARIANT(None);

template<class T>
constexpr Option<typename RemoveConstOrRef<T>::Result> make_some(T &&value) {
  return Some<typename RemoveConstOrRef<T>::Result>{forward<T>(value)};
}

template<class T>
constexpr Option<typename RemoveConstOrRef<T>::Result> make_none() {
  return None{};
}

template<class T>
template<class U, class F>
crust_cxx14_constexpr Option<U>
Option<T>::map(ops::Fn<F, U(const T &)> f) const {
  return this->template visit<Option<U>>(
      [&](const Some<T> &value) {
        return make_some(f(value.template get<0>()));
      },
      [](const None &) { return make_none<U>(); }
  );
}

template<class T>
crust_cxx14_constexpr Option<T> Option<T>::take() {
  auto tmp = this->template move_variant<Option<T>>();
  *this = None{};
  return tmp;
}
}

using option::make_some;
using option::make_none;
}


#endif //_CRUST_INCLUDE_OPTION_HPP
