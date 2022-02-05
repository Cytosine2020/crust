#ifndef _CRUST_INCLUDE_ENUM_HPP
#define _CRUST_INCLUDE_ENUM_HPP


#include "enum_decl.hpp"

#include "utility.hpp"
#include "option.hpp"


namespace crust {
namespace _impl_enum {
template<class... Fields>
template<class T>
crust_cxx14_constexpr Option<T> EnumTagUnion<Fields...>::move_variant() {
  constexpr usize i = IndexGetter<typename RemoveConstOrRef<T>::Result>::result;
  return index == i ?
      make_some(EnumGetter<i, _trivial, Fields...>::inner(holder)) :
      None{};
}

template<class... Fields>
template<class T>
crust_cxx14_constexpr Option<T> EnumTagOnly<Fields...>::move_variant() {
  constexpr usize i = IndexGetter<typename RemoveConstOrRef<T>::Result>::result;
  return index == i ? make_some(T{}) : None{};
}
}

template<class... Fields>
template<class T>
crust_cxx14_constexpr Option<T> Enum<Fields...>::move_variant() {
  return inner.template move_variant<T>();
}
}


#endif //_CRUST_INCLUDE_ENUM_HPP
