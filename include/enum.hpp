#ifndef _CRUST_INCLUDE_ENUM_HPP
#define _CRUST_INCLUDE_ENUM_HPP


#include "enum_decl.hpp"

#include "cmp.hpp"
#include "option.hpp"
#include "tuple.hpp"
#include "utility.hpp"


namespace crust {
namespace _impl_enum {
template <class... Fields>
template <class T>
crust_cxx14_constexpr Option<T> EnumTagUnion<Fields...>::move_variant() {
  constexpr usize i = IndexGetter<typename RemoveConstOrRef<T>::Result>::result;
  return index == i ?
      make_some(EnumGetter<i, trivial, Fields...>::inner(holder)) :
      None{};
}

template <class... Fields>
template <class T>
crust_cxx14_constexpr Option<T> EnumTagOnly<Fields...>::move_variant() {
  constexpr usize i = IndexGetter<typename RemoveConstOrRef<T>::Result>::result;
  return index == i ? make_some(T{}) : None{};
}
} // namespace _impl_enum

template <class... Fields>
template <class T>
crust_cxx14_constexpr Option<T> Enum<Fields...>::move_variant() {
  return inner.template move_variant<T>();
}

namespace _impl_enum {
template <class T, class... Fields>
struct LetEnum {
private:
  _impl_tuple::TupleHolder<Fields &...> ref;

public:
  explicit constexpr LetEnum(Fields &...ref) : ref{ref...} {}

  template <class... Vs>
  crust_cxx14_constexpr bool operator=(Enum<Vs...> &&other) {
    return other.inner.template let_helper<T>(ref);
  }
};
} // namespace _impl_enum

template <class T, class... Fields>
crust_cxx14_constexpr _impl_enum::LetEnum<T, Fields...> let(Fields &...fields) {
  return _impl_enum::LetEnum<T, Fields...>{fields...};
}
} // namespace crust


#endif //_CRUST_INCLUDE_ENUM_HPP
