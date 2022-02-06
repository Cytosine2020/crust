#ifndef _CRUST_INCLUDE_ENUM_HPP
#define _CRUST_INCLUDE_ENUM_HPP


#include "enum_decl.hpp"

#include "utility.hpp"
#include "cmp.hpp"
#include "tuple.hpp"
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

template<class ...Fields>
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

namespace _impl_enum {
template<class T, class ...Fields>
struct LetEnum {
  _impl_tuple::TupleHolder<Fields &...> ref;

  explicit constexpr LetEnum(Fields &...ref) : ref{ref...} {}

  template<class ...Vs>
  crust_cxx14_constexpr bool operator=(Enum<Vs...> &&enum_) {
    return enum_.inner.template let_helper<T>(ref);
  }
};
}

template<class T, class ...Fields>
crust_cxx14_constexpr _impl_enum::LetEnum<T, Fields...>
let(Fields &...fields) {
  return _impl_enum::LetEnum<T, Fields...>{fields...};
}
}


#endif //_CRUST_INCLUDE_ENUM_HPP
