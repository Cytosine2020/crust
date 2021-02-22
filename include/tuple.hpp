#ifndef CRUST_TUPLE_HPP
#define CRUST_TUPLE_HPP


#include "tuple_declare.hpp"

#include "option.hpp"
#include "cmp.hpp"


namespace crust {
namespace __impl_tuple {
template<class Field, class ...Fields>
constexpr Option<Ordering>
TupleHolder<Field, Fields...>::partial_cmp(const TupleHolder &other) const {
    return operator_partial_cmp(this->field, other.field).map(make_fn([&](Ordering value) {
        return value.then(operator_partial_cmp(this->remains, other.remains));
    }));
}

template<class Field, class ...Fields>
constexpr Ordering TupleHolder<Field, Fields...>::cmp(const TupleHolder &other) const {
    return operator_cmp(this->field, other.field).then(
            operator_cmp(this->remains, other.remains)
    );
}

template<class Field>
constexpr Option<Ordering> TupleHolder<Field>::partial_cmp(const TupleHolder &other) const {
    return operator_partial_cmp(this->field, other.field);
}

template<class Field>
constexpr Ordering TupleHolder<Field>::cmp(const TupleHolder &other) const {
    return operator_cmp(this->field, other.field);
}

inline constexpr Option<Ordering> TupleHolder<>::partial_cmp(const TupleHolder &) const {
    return make_some(Ordering::equal());
}

inline constexpr Ordering TupleHolder<>::cmp(const TupleHolder &) const {
    return Ordering::equal();
}
}

template<class ...Fields>
constexpr Option<Ordering> Tuple<Fields...>::partial_cmp(const Tuple &other) const {
    return this->holder.partial_cmp(other.holder);
}

template<class ...Fields>
constexpr Ordering Tuple<Fields...>::cmp(const Tuple &other) const {
    return this->holder.cmp(other.holder);
}
}


#endif //CRUST_TUPLE_HPP
