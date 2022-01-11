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
struct CRUST_EBCO TupleHolder<Field, Fields...> : public Impl<
        cmp::PartialEq<TupleHolder<Field, Fields...>>,
        Derive<Field, cmp::PartialEq>::result,
        Derive<TupleHolder<Fields...>, cmp::PartialEq>::result
>, public Impl<
        cmp::Eq<TupleHolder<Field, Fields...>>,
        Derive<Field, cmp::Eq>::result,
        Derive<TupleHolder<Fields...>, cmp::Eq>::result
>, public Impl<
        cmp::PartialOrd<TupleHolder<Field, Fields...>>,
        Derive<Field, cmp::PartialOrd>::result,
        Derive<TupleHolder<Fields...>, cmp::PartialOrd>::result
>, public Impl<
        cmp::Ord<TupleHolder<Field, Fields...>>,
        Derive<Field, cmp::Ord>::result,
        Derive<TupleHolder<Fields...>, cmp::Ord>::result
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

    constexpr Option<cmp::Ordering> partial_cmp(const TupleHolder &other) const;

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

    constexpr cmp::Ordering cmp(const TupleHolder &other) const;
};

template<class Field>
struct CRUST_EBCO TupleHolder<Field> :
        public Impl<cmp::PartialEq<TupleHolder<Field>>, Derive<Field, cmp::PartialEq>::result>,
        public Impl<cmp::Eq<TupleHolder<Field>>, Derive<Field, cmp::Eq>::result>,
        public Impl<cmp::PartialOrd<TupleHolder<Field>>, Derive<Field, cmp::PartialOrd>::result>,
        public Impl<cmp::Ord<TupleHolder<Field>>, Derive<Field, cmp::Ord>::result> {
    Field field;

    constexpr TupleHolder() noexcept: field{} {}

    template<class T>
    explicit constexpr TupleHolder(T &&field) noexcept: field{forward<T>(field)} {}

    /// impl PartialEq

    constexpr bool eq(const TupleHolder &other) const { return field == other.field; }

    constexpr bool ne(const TupleHolder &other) const { return field != other.field; }

    /// impl PartialOrd

    constexpr Option<cmp::Ordering> partial_cmp(const TupleHolder &other) const;

    constexpr bool lt(const TupleHolder &other) const { return field < other.field; }

    constexpr bool le(const TupleHolder &other) const { return field <= other.field; }

    constexpr bool gt(const TupleHolder &other) const { return field > other.field; }

    constexpr bool ge(const TupleHolder &other) const { return field >= other.field; }

    /// impl Ord

    constexpr cmp::Ordering cmp(const TupleHolder &other) const;
};

template<>
struct CRUST_EBCO TupleHolder<> :
        public cmp::PartialEq<TupleHolder<>>, public cmp::Eq<TupleHolder<>>,
        public cmp::PartialOrd<TupleHolder<>>, public cmp::Ord<TupleHolder<>> {
    /// impl PartialEq

    constexpr bool eq(const TupleHolder &) const { return true; }

    /// impl PartialOrd

    constexpr Option<cmp::Ordering> partial_cmp(const TupleHolder &) const;

    /// impl Ord

    constexpr cmp::Ordering cmp(const TupleHolder &) const;
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
};

template<class Field, class ...Fields>
struct TupleGetter<0, Field, Fields...> {
    using Self = TupleHolder<Field, Fields...>;
    using Result = Field;

    static constexpr const Result &inner(const Self &self) { return self.field; }

    static constexpr Result &inner(Self &self) { return self.field; }
};
}

template<class T>
class Option;

template<class ...Fields>
class CRUST_EBCO Tuple : public Impl<
        cmp::PartialEq<Tuple<Fields...>>,
        Derive<__impl_tuple::TupleHolder<Fields...>, cmp::PartialEq>::result
>, public Impl<
        cmp::Eq<Tuple<Fields...>>,
        Derive<__impl_tuple::TupleHolder<Fields...>, cmp::Eq>::result
>, public Impl<
        cmp::PartialOrd<Tuple<Fields...>>,
        Derive<__impl_tuple::TupleHolder<Fields...>, cmp::PartialOrd>::result
>, public Impl<
        cmp::Ord<Tuple<Fields...>>,
        Derive<__impl_tuple::TupleHolder<Fields...>, cmp::Ord>::result
>, public Impl<MonoStateTag, sizeof...(Fields) == 0> {
private:
    using __Holder = __impl_tuple::TupleHolder<typename RemoveRef<Fields>::Result...>;
    template<usize index> using __Getter = __impl_tuple::TupleGetter<index, Fields...>;

    __Holder holder;

public:
    template<usize index> using __Result = typename __Getter<index>::Result;

    static constexpr usize __size = sizeof...(Fields);

    constexpr Tuple() noexcept: holder{} {}

    template<class ...Ts>
    explicit constexpr Tuple(Ts &&...fields) noexcept: holder{forward<Ts>(fields)...} {}

    template<usize index>
    constexpr const __Result<index> &get() const { return __Getter<index>::inner(holder); }

    template<usize index>
    CRUST_CXX14_CONSTEXPR __Result<index> &get() { return __Getter<index>::inner(holder); }

    /// impl PartialEq

    constexpr bool eq(const Tuple &other) const { return holder == other.holder; }

    constexpr bool ne(const Tuple &other) const { return holder != other.holder; }

    /// impl PartialOrd

    constexpr Option<cmp::Ordering> partial_cmp(const Tuple &other) const;

    constexpr bool lt(const Tuple &other) const { return holder < other.holder; }

    constexpr bool le(const Tuple &other) const { return holder <= other.holder; }

    constexpr bool gt(const Tuple &other) const { return holder > other.holder; }

    constexpr bool ge(const Tuple &other) const { return holder >= other.holder; }

    /// impl Ord

    constexpr cmp::Ordering cmp(const Tuple &other) const;
};

template<>
struct IsMonoState<Tuple<>> {
    static constexpr bool result = true;
};

template<class ...Fields>
constexpr Tuple<Fields...> make_tuple(Fields &&...fields) {
    return Tuple<Fields...>{forward<Fields>(fields)...};
}


namespace __impl_tuple {
template<usize index, class ...Fields>
struct TupleEqHelper {
    static constexpr bool inner(TupleHolder<Fields &...> &ref, Tuple<Fields...> &tuple) {
        return TupleEqHelper<index - 1, Fields...>::inner(ref, tuple) &&
               TupleGetter<index - 1, Fields &...>::inner(ref) == tuple.template get<index - 1>();
    }
};

template<class ...Fields>
struct TupleEqHelper<0, Fields...> {
    static CRUST_CXX14_CONSTEXPR bool inner(TupleHolder<Fields &...> &, Tuple<Fields...> &) {
        return true;
    }
};


template<usize index, class ...Fields>
struct LetTupleHelper {
    static CRUST_CXX14_CONSTEXPR void
    inner(TupleHolder<Fields &...> &ref, Tuple<Fields...> &&tuple) {
        TupleGetter<index - 1, Fields &...>::inner(ref) = move(tuple.template get<index - 1>());
        LetTupleHelper<index - 1, Fields...>::inner(ref, move(tuple));
    }
};

template<class ...Fields>
struct LetTupleHelper<0, Fields...> {
    static CRUST_CXX14_CONSTEXPR void inner(TupleHolder<Fields &...> &, Tuple<Fields...> &&) {}
};


template<class ...Fields>
struct LetTuple {
    TupleHolder<Fields &...> ref;

    explicit constexpr LetTuple(Fields &...fields) : ref{fields...} {}

    CRUST_CXX14_CONSTEXPR void operator=(Tuple<Fields...> &&tuple) {
        LetTupleHelper<sizeof...(Fields), Fields...>::inner(ref, move(tuple));
    }
};
}

template<class ...Fields>
CRUST_CXX14_CONSTEXPR __impl_tuple::LetTuple<typename RemoveRef<Fields>::Result...>
let(Fields &&...fields) {
    return __impl_tuple::LetTuple<typename RemoveRef<Fields>::Result...>{
            forward<Fields>(fields)...
    };
}
}


namespace std {

/// c++ std bindings

template<class ...Fields>
struct tuple_size<crust::Tuple<Fields...>> :
        public integral_constant<crust::usize, crust::Tuple<Fields...>::__size> {
};

template<crust::usize index, class ...Fields>
struct tuple_element<index, crust::Tuple<Fields...>> {
    using type = typename crust::Tuple<Fields...>::template __Result<index>;
};

template<crust::usize index, class ...Fields>
constexpr const typename tuple_element<index, crust::Tuple<Fields...>>::type &
get(const crust::Tuple<Fields...> &object) noexcept {
    return object.template get<index>();
}

template<crust::usize index, class ...Fields>
constexpr typename tuple_element<index, crust::Tuple<Fields...>>::type &
get(crust::Tuple<Fields...> &object) noexcept {
    return object.template get<index>();
}
}


#endif //CRUST_TUPLE_DECLARE_HPP
