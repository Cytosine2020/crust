#ifndef CRUST_TUPLE_DECLARE_HPP
#define CRUST_TUPLE_DECLARE_HPP


#include <utility>

#include "utility.hpp"
#include "cmp_declare.hpp"


namespace crust {
template<class T>
class Option;

namespace __impl_tuple {
template<class ...Fields>
struct TupleHolder;

template<class Field, class ...Fields>
struct TupleHolder<Field, Fields...> : public Impl<
        PartialEq<TupleHolder<Field, Fields...>>,
        CRUST_DERIVE(Field, PartialEq),
        CRUST_DERIVE(TupleHolder<Fields...>, PartialEq)
>, public Impl<
        Eq<TupleHolder<Field, Fields...>>,
        CRUST_DERIVE(Field, Eq),
        CRUST_DERIVE(TupleHolder<Fields...>, Eq)
>, public Impl<
        PartialOrd<TupleHolder<Field, Fields...>>,
        CRUST_DERIVE(Field, PartialOrd),
        CRUST_DERIVE(TupleHolder<Fields...>, PartialOrd)
>, public Impl<
        Ord<TupleHolder<Field, Fields...>>,
        CRUST_DERIVE(Field, Ord),
        CRUST_DERIVE(TupleHolder<Fields...>, Ord)
> {
    Field field;
    TupleHolder<Fields...> remains;

    constexpr TupleHolder() noexcept: field{}, remains{} {}

    template<class T, class ...Ts>
    explicit constexpr TupleHolder(T &&field, Ts &&...fields) noexcept:
            field{forward<T>(field)}, remains{forward<Ts>(fields)...} {}

    /// impl PartialEq

    constexpr bool eq(const TupleHolder &other) const {
        return field == other.field && remains == other.remains;
    }

    constexpr bool ne(const TupleHolder &other) const {
        return field != other.field || remains != other.remains;
    }

    /// impl PartialOrd

constexpr Option<Ordering> partial_cmp(const TupleHolder &other) const;

    constexpr bool lt(const TupleHolder &other) const {
        return field != other.field ? field < other.field : remains < other.remains;
    }

    constexpr bool le(const TupleHolder &other) const {
        return field != other.field ? field <= other.field : remains <= other.remains;
    }

    constexpr bool gt(const TupleHolder &other) const {
        return field != other.field ? field > other.field : remains > other.remains;
    }

    constexpr bool ge(const TupleHolder &other) const {
        return field != other.field ? field >= other.field : remains >= other.remains;
    }

    /// impl Ord

    constexpr Ordering cmp(const TupleHolder &other) const;
};

template<class Field>
struct TupleHolder<Field> :
        public Impl<PartialEq<TupleHolder<Field>>, CRUST_DERIVE(Field, PartialEq)>,
        public Impl<Eq<TupleHolder<Field>>, CRUST_DERIVE(Field, Eq)>,
        public Impl<PartialOrd<TupleHolder<Field>>, CRUST_DERIVE(Field, PartialOrd)>,
        public Impl<Ord<TupleHolder<Field>>, CRUST_DERIVE(Field, Ord)> {
    Field field;

    constexpr TupleHolder() noexcept: field{} {}

    template<class T>
    explicit constexpr TupleHolder(T &&field) noexcept: field{forward<T>(field)} {}

    /// impl PartialEq

    constexpr bool eq(const TupleHolder &other) const { return field == other.field; }

    constexpr bool ne(const TupleHolder &other) const { return field != other.field; }

    /// impl PartialOrd

    constexpr Option<Ordering> partial_cmp(const TupleHolder &other) const;

    constexpr bool lt(const TupleHolder &other) const { return field < other.field; }

    constexpr bool le(const TupleHolder &other) const { return field <= other.field; }

    constexpr bool gt(const TupleHolder &other) const { return field > other.field; }

    constexpr bool ge(const TupleHolder &other) const { return field >= other.field; }

    /// impl Ord

    constexpr Ordering cmp(const TupleHolder &other) const;
};

template<>
struct TupleHolder<> :
        public PartialEq<TupleHolder<>>, public Eq<TupleHolder<>>,
        public PartialOrd<TupleHolder<>>, public Ord<TupleHolder<>> {
    /// impl PartialEq

    constexpr bool eq(const TupleHolder &) const { return true; }

    /// impl PartialOrd

    constexpr Option<Ordering> partial_cmp(const TupleHolder &) const;

    /// impl Ord

    constexpr Ordering cmp(const TupleHolder &) const;
};

template<usize index, class ...Fields>
struct TupleGetter;

template<usize index, class Field, class ...Fields>
struct TupleGetter<index, Field, Fields...> {
    using Self = TupleHolder<Field, Fields...>;
    using Result = typename TupleGetter<index - 1, Fields...>::Result;

    static constexpr const Result &inner(const Self &self) {
        return TupleGetter<index - 1, Fields...>::inner(self.remains);
    }

    static constexpr Result &inner(Self &self) {
        return TupleGetter<index - 1, Fields...>::inner(self.remains);
    }

    static constexpr const Result &&inner_move(const Self &&self) {
        return TupleGetter<index - 1, Fields...>::inner_move(move(self.remains));
    }

    static constexpr Result &&inner_move(Self &&self) {
        return TupleGetter<index - 1, Fields...>::inner_move(move(self.remains));
    }
};

template<class Field, class ...Fields>
struct TupleGetter<0, Field, Fields...> {
    using Self = TupleHolder<Field, Fields...>;
    using Result = Field;

    static constexpr const Result &inner(const Self &self) { return self.field; }

    static constexpr Result &inner(Self &self) { return self.field; }

    static constexpr const Result &&inner_move(const Self &&self) { return move(self.field); }

    static constexpr Result &&inner_move(Self &&self) { return move(self.field); }
};
}

template<class T>
class Option;

template<class ...Fields>
struct Tuple : public Impl<
        PartialEq<Tuple<Fields...>>,
        CRUST_DERIVE(__impl_tuple::TupleHolder<Fields...>, PartialEq)
>, public Impl<
        Eq<Tuple<Fields...>>,
        CRUST_DERIVE(__impl_tuple::TupleHolder<Fields...>, Eq)
>, public Impl<
        PartialOrd<Tuple<Fields...>>,
        CRUST_DERIVE(__impl_tuple::TupleHolder<Fields...>, PartialOrd)
>, public Impl<
        Ord<Tuple<Fields...>>,
        CRUST_DERIVE(__impl_tuple::TupleHolder<Fields...>, Ord)
> {
private:
    using __Holder = __impl_tuple::TupleHolder<typename RemoveRef<Fields>::Result...>;
    template<usize index> using __Getter = __impl_tuple::TupleGetter<index, Fields...>;

    __Holder holder;

public:
    template<usize index> using __Result = typename __Getter<index>::Result;

    constexpr Tuple() noexcept: holder{} {}

    template<class ...Ts>
    explicit constexpr Tuple(Ts &&...fields) noexcept: holder{forward<Ts>(fields)...} {}

    template<usize index>
    constexpr const __Result<index> &get() const { return __Getter<index>::inner(holder); }

    template<usize index>
    CRUST_CXX14_CONSTEXPR __Result<index> &get() { return __Getter<index>::inner(holder); }

    template<usize index>
    constexpr const __Result<index> &&move() const {
        return __Getter<index>::inner_move(move(holder));
    }

    template<usize index>
    CRUST_CXX14_CONSTEXPR __Result<index> &&move() {
        return __Getter<index>::inner_move(move(holder));
    }

    /// impl PartialEq

    constexpr bool eq(const Tuple &other) const { return holder == other.holder; }

    constexpr bool ne(const Tuple &other) const { return holder != other.holder; }

    /// impl PartialOrd

    constexpr Option<Ordering> partial_cmp(const Tuple &other) const;

    constexpr bool lt(const Tuple &other) const { return holder < other.holder; }

    constexpr bool le(const Tuple &other) const { return holder <= other.holder; }

    constexpr bool gt(const Tuple &other) const { return holder > other.holder; }

    constexpr bool ge(const Tuple &other) const { return holder >= other.holder; }

    /// impl Ord

    constexpr Ordering cmp(const Tuple &other) const;
};

template<class ...Fields>
constexpr Tuple<Fields...> make_tuple(Fields &&...fields) {
    return Tuple<Fields...>{forward<Fields>(fields)...};
}
}


namespace std {

/// c++ std bindings

template<class ...Fields>
struct tuple_size<crust::Tuple<Fields...>> : integral_constant<crust::usize, sizeof...(Fields)> {
};

template<crust::usize index, class ...Fields>
struct tuple_element<index, crust::Tuple<Fields...>> {
    using type = typename crust::Tuple<Fields...>::template __Result<index>;
};

template<crust::usize index, class ...Fields>
constexpr const class tuple_element<index, crust::Tuple<Fields...>>::type &
get(const crust::Tuple<Fields...> &object) noexcept {
    return object.template get<index>();
}

template<crust::usize index, class ...Fields>
constexpr typename tuple_element<index, crust::Tuple<Fields...>>::type &
get(crust::Tuple<Fields...> &object) noexcept {
    return object.template get<index>();
}

template<crust::usize index, class ...Fields>
constexpr const class tuple_element<index, crust::Tuple<Fields...>>::type &&
get(const crust::Tuple<Fields...> &&object) noexcept {
    return object.template move<index>();
}

template<crust::usize index, class ...Fields>
constexpr typename tuple_element<index, crust::Tuple<Fields...>>::type &&
get(crust::Tuple<Fields...> &&object) noexcept {
    return object.template move<index>();
}
}


#endif //CRUST_TUPLE_DECLARE_HPP
