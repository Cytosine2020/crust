#ifndef CRUST_ENUM_HPP
#define CRUST_ENUM_HPP


#include "crust/enum_decl.hpp"

#include "crust/cmp.hpp"
#include "crust/option.hpp"
#include "crust/utility.hpp"


namespace crust {
namespace _impl_enum {
template <class Inner, class... Fields>
template <class T>
crust_cxx14_constexpr Option<T> Enum<Inner, Fields...>::move_variant() && {
  return is_variant<T>() ?
      make_some(move(inner.template unsafe_get_variant<T>())) :
      None{};
}

template <class T>
struct LetEnum {
private:
  T &ref;

public:
  explicit constexpr LetEnum(T &ref) : ref{ref} {}

  template <class... Fields>
  crust_cxx14_constexpr bool operator=(Enum<Fields...> &&other) {
    return other.inner.template let_helper<T>(ref);
  }
};
} // namespace _impl_enum

template <class T>
crust_cxx14_constexpr _impl_enum::LetEnum<T> let(T &ref) {
  return _impl_enum::LetEnum<T>{ref};
}

// namespace _auto_impl {
// template <class Self>
// constexpr Option<cmp::Ordering>
// EnumPartialOrdImpl<Self>::partial_cmp(const Self &other) const {
//   return self()._partial_cmp(other);
// }
// } // namespace _auto_impl
} // namespace crust


#endif // CRUST_ENUM_HPP
