#ifndef CRUST_RESULT_HPP
#define CRUST_RESULT_HPP


#include "utility.hpp"

#include "enum.hpp"
#include "option.hpp"


namespace crust {
template<class T>
CRUST_ENUM_VARIANT(Ok, T);

template<class E>
CRUST_ENUM_VARIANT(Err, E);

template<class T, class E>
class Result : public Enum<Ok<T>, Err<E>> {
public:
    CRUST_ENUM_USE_BASE(Result, Enum<Ok<T>, Err<E>>);

    constexpr bool is_ok() const { return this->template is_variant<Ok<T>>(); }

    constexpr bool is_err() const { return this->template is_variant<Err<E>>(); }

    constexpr bool contains(const T &other) const {
        return this->template eq_variant<T>(other);
    }

    constexpr bool contains_err(const E &other) const {
        return this->template eq_variant<E>(other);
    }

    CRUST_CXX14_CONSTEXPR Option<T> ok() { return this->template move_variant<Option<T>>(); }

    CRUST_CXX14_CONSTEXPR Option<E> err() { return this->template move_variant<Option<E>>(); }
};
}


#endif //CRUST_RESULT_HPP
