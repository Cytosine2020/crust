#ifndef CRUST_OPTION_HPP
#define CRUST_OPTION_HPP


#include "crust/option_decl.hpp"


namespace crust {
namespace option {
CRUST_ENUM_VARIANT(None);

template <class T>
CRUST_ENUM_TUPLE_VARIANT(Some, Some<T>, T);

template <class T>
crust_always_inline constexpr Option<typename RemoveConstOrRefType<T>::Result>
make_some(T &&value) {
  return Some<typename RemoveConstOrRefType<T>::Result>{forward<T>(value)};
}

template <class T>
crust_always_inline constexpr Option<typename RemoveConstOrRefType<T>::Result>
make_none() {
  return None{};
}

template <class T>
template <class U, class F>
constexpr Option<U> Option<T>::map(ops::Fn<F, U(const T &)> f) const {
  return this->template visit<Option<U>>(
      [&](const Some<T> &value) {
        return make_some(f(value.template get<0>()));
      },
      [](const None &) { return make_none<U>(); });
}

template <class T>
crust_cxx14_constexpr Option<T> Option<T>::take() {
  auto tmp = this->template move_variant<Option<T>>();
  *this = None{};
  return tmp;
}
} // namespace option

using option::make_none;
using option::make_some;
} // namespace crust


#endif // CRUST_OPTION_HPP
