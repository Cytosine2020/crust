#ifndef CRUST_ENUM_HPP
#define CRUST_ENUM_HPP


#include "enum_declare.hpp"

#include "utility.hpp"
#include "option.hpp"


namespace crust {
namespace __impl_enum {
template<class... Fields>
template<class T>
CRUST_CXX14_CONSTEXPR Option<T> EnumTagUnion<Fields...>::move_variant() {
  constexpr usize i = __IndexGetter<typename RemoveRef<T>::Result>::result;
  return index == i ?
      make_some(EnumGetter<i, __trivial, Fields...>::inner(holder)) :
      None{};
}

template<class... Fields>
template<class T>
CRUST_CXX14_CONSTEXPR Option<T> EnumTagOnly<Fields...>::move_variant() {
  constexpr usize i = __IndexGetter<typename RemoveRef<T>::Result>::result;
  return index == i ? make_some(T{}) : None{};
}
}

template<class... Fields>
template<class T>
CRUST_CXX14_CONSTEXPR Option<T> Enum<Fields...>::move_variant() {
  return inner.template move_variant<T>();
}
}


#endif //CRUST_ENUM_HPP
