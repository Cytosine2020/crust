#ifndef _CRUST_INCLUDE_TUPLE_HPP
#define _CRUST_INCLUDE_TUPLE_HPP


#include "tuple_decl.hpp"

#include "option.hpp"
#include "cmp.hpp"


namespace crust {
namespace _impl_tuple {
template<class Field, class ...Fields>
constexpr Option<cmp::Ordering>
TupleHolder<Field, Fields...>::partial_cmp(const TupleHolder &other) const {
  return cmp::operator_partial_cmp(this->field, other.field).map(bind(
      [&](cmp::Ordering value) {
        return value.then(
            cmp::operator_partial_cmp(this->remains, other.remains)
        );
      }
  ));
}

template<class Field, class ...Fields>
constexpr cmp::Ordering
TupleHolder<Field, Fields...>::cmp(const TupleHolder &other) const {
  return cmp::operator_cmp(this->field, other.field).then(
      cmp::operator_cmp(this->remains, other.remains)
  );
}

template<class Field>
constexpr Option<cmp::Ordering>
TupleHolder<Field>::partial_cmp(const TupleHolder &other) const {
  return cmp::operator_partial_cmp(this->field, other.field);
}

template<class Field>
constexpr cmp::Ordering
TupleHolder<Field>::cmp(const TupleHolder &other) const {
  return cmp::operator_cmp(this->field, other.field);
}

inline constexpr Option<cmp::Ordering>
TupleHolder<>::partial_cmp(const TupleHolder &) const {
  return make_some(cmp::make_equal());
}

inline constexpr cmp::Ordering TupleHolder<>::cmp(const TupleHolder &) const {
  return cmp::make_equal();
}
}

template<class ...Fields>
constexpr Option<cmp::Ordering>
Tuple<Fields...>::partial_cmp(const Tuple &other) const {
  return this->holder.partial_cmp(other.holder);
}

template<class ...Fields>
constexpr cmp::Ordering Tuple<Fields...>::cmp(const Tuple &other) const {
  return this->holder.cmp(other.holder);
}
}


#endif //_CRUST_INCLUDE_TUPLE_HPP
