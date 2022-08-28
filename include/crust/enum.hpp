#ifndef CRUST_ENUM_HPP
#define CRUST_ENUM_HPP


#include "crust/enum_decl.hpp"

#include "crust/cmp.hpp"
#include "crust/option.hpp"
#include "crust/utility.hpp"


namespace crust {
namespace _impl_enum {
template <class Index, class... Fields>
template <class T>
constexpr Option<cmp::Ordering>
EnumTagUnion<Index, Fields...>::PartialCmp::operator()(const T &value) const {
  return operator_partial_cmp(value, other->unsafe_get_variant<T>());
}

template <class Index, class... Fields>
template <class T>
constexpr cmp::Ordering
EnumTagUnion<Index, Fields...>::Cmp::operator()(const T &value) const {
  return operator_cmp(value, other->unsafe_get_variant<T>());
}

template <class Index, class... Fields>
constexpr Option<cmp::Ordering>
EnumTagUnion<Index, Fields...>::partial_cmp(const EnumTagUnion &other) const {
  return make_some(operator_cmp(*this, other)); // FIXME:
}

template <class Index, class... Fields>
constexpr cmp::Ordering
EnumTagUnion<Index, Fields...>::cmp(const EnumTagUnion &other) const {
  return operator_cmp(index, other.index)
      .then(visit<cmp::Ordering>(Cmp{&other}));
}

template <class Index, class... Fields>
constexpr Option<cmp::Ordering>
EnumTagOnly<Index, Fields...>::partial_cmp(const EnumTagOnly &other) const {
  return operator_partial_cmp(index, other.index);
}

template <class Index, class... Fields>
constexpr cmp::Ordering
EnumTagOnly<Index, Fields...>::cmp(const EnumTagOnly &other) const {
  return operator_cmp(index, other.index);
}

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

template <class S>
constexpr Option<cmp::Ordering> ImplFor<
    Trait<cmp::PartialOrd>,
    S,
    EnableIf<_impl_derive::
                 ImplForEnum<typename BluePrint<S>::Result, cmp::PartialOrd>>>::
    partial_cmp(const Self &other) const {
  return operator_partial_cmp(self().inner, other.inner);
}

template <class S>
constexpr cmp::Ordering ImplFor<
    Trait<cmp::Ord>,
    S,
    EnableIf<
        _impl_derive::ImplForEnum<typename BluePrint<S>::Result, cmp::Ord>>>::
    cmp(const Self &other) const {
  return operator_cmp(self().inner, other.inner);
}
} // namespace crust


#endif // CRUST_ENUM_HPP
