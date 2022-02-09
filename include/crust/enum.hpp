#ifndef CRUST_ENUM_HPP
#define CRUST_ENUM_HPP


#include "crust/enum_decl.hpp"

#include "crust/cmp.hpp"
#include "crust/option.hpp"
#include "crust/utility.hpp"


namespace crust {
namespace _impl_enum {
template <class... Fields>
template <class T>
crust_cxx14_constexpr Option<T> EnumTagUnion<Fields...>::move_variant() {
  constexpr usize i =
      IndexGetter<typename RemoveConstOrRefType<T>::Result>::result;
  return index == i ?
      make_some(EnumGetter<i, trivial, Fields...>::inner(holder)) :
      None{};
}

template <class... Fields>
template <class T>
crust_cxx14_constexpr Option<T> EnumTagOnly<Fields...>::move_variant() {
  constexpr usize i =
      IndexGetter<typename RemoveConstOrRefType<T>::Result>::result;
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


#endif // CRUST_ENUM_HPP
