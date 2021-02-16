#ifndef CRUST_TUPLE_HPP
#define CRUST_TUPLE_HPP


#include <utility>

#include "utility.hpp"
#include "cmp.hpp"
#include "option.hpp"


namespace crust {
namespace __impl {
template<class ...Entries>
struct TupleHolder;

template<class Entry, class ...Entries>
struct TupleHolder<Entry, Entries...> : public Impl<
        PartialEq<TupleHolder<Entry, Entries...>>,
        CRUST_DERIVE(Entry, PartialEq),
        CRUST_DERIVE(TupleHolder<Entries...>, PartialEq)
>, public Impl<
        Eq<TupleHolder<Entry, Entries...>>,
        CRUST_DERIVE(Entry, Eq),
        CRUST_DERIVE(TupleHolder<Entries...>, Eq)
>, public Impl<
        PartialOrd<TupleHolder<Entry, Entries...>>,
        CRUST_DERIVE(Entry, PartialOrd),
        CRUST_DERIVE(TupleHolder<Entries...>, PartialOrd)
>, public Impl<
        Ord<TupleHolder<Entry, Entries...>>,
        CRUST_DERIVE(Entry, Ord),
        CRUST_DERIVE(TupleHolder<Entries...>, Ord)
> {
    Entry entry;
    TupleHolder<Entries...> remains;

    template<class T, class ...Ts>
    explicit constexpr TupleHolder(T &&entry, Ts &&...entries) noexcept:
            entry{forward<T>(entry)}, remains{forward<Ts>(entries)...} {}

    /// impl PartialEq

    bool eq(const TupleHolder &other) const {
        return this->entry == other.entry && this->remains == other.remains;
    }

    bool ne(const TupleHolder &other) const {
        return this->entry != other.entry || this->remains != other.remains;
    }

    /// impl PartialOrd

    Option<Ordering> partial_cmp(const TupleHolder &other) const {
        return operator_partial_cmp(this->entry, other.entry).map(make_fn([&](Ordering value) {
            return value.then(operator_partial_cmp(this->remains, other.remains));
        }));
    }

    bool lt(const TupleHolder &other) const {
        if (this->entry != other.entry) {
            return this->entry < other.entry;
        } else {
            return this->remains < other.remains;
        }
    }

    bool le(const TupleHolder &other) const {
        if (this->entry != other.entry) {
            return this->entry <= other.entry;
        } else {
            return this->remains <= other.remains;
        }
    }

    bool gt(const TupleHolder &other) const {
        if (this->entry != other.entry) {
            return this->entry > other.entry;
        } else {
            return this->remains > other.remains;
        }
    }

    bool ge(const TupleHolder &other) const {
        if (this->entry != other.entry) {
            return this->entry >= other.entry;
        } else {
            return this->remains >= other.remains;
        }
    }

    /// impl Ord

    Ordering cmp(const TupleHolder &other) const {
        return operator_cmp(this->entry, other.entry).then(
                operator_cmp(this->remains, other.remains)
        );
    }
};

template<class Entry>
struct TupleHolder<Entry> :
        public Impl<PartialEq<TupleHolder<Entry>>, CRUST_DERIVE(Entry, PartialEq)>,
        public Impl<Eq<TupleHolder<Entry>>, CRUST_DERIVE(Entry, Eq)>,
        public Impl<PartialOrd<TupleHolder<Entry>>, CRUST_DERIVE(Entry, PartialOrd)>,
        public Impl<Ord<TupleHolder<Entry>>, CRUST_DERIVE(Entry, Ord)> {
    Entry entry;

    template<class T>
    explicit constexpr TupleHolder(T &&entry) noexcept: entry{forward<T>(entry)} {}

    /// impl PartialEq

    bool eq(const TupleHolder &other) const { return this->entry == other.entry; }

    bool ne(const TupleHolder &other) const { return this->entry != other.entry; }

    /// impl PartialOrd

    Option<Ordering> partial_cmp(const TupleHolder &other) const {
        return operator_partial_cmp(this->entry, other.entry);
    }

    bool lt(const TupleHolder &other) const { return this->entry < other.entry; }

    bool le(const TupleHolder &other) const { return this->entry <= other.entry; }

    bool gt(const TupleHolder &other) const { return this->entry > other.entry; }

    bool ge(const TupleHolder &other) const { return this->entry >= other.entry; }

    /// impl Ord

    Ordering cmp(const TupleHolder &other) const { return operator_cmp(this->entry, other.entry); }
};

template<>
struct TupleHolder<> :
        public PartialEq<TupleHolder<>>, public Eq<TupleHolder<>>,
        public PartialOrd<TupleHolder<>>, public Ord<TupleHolder<>> {
    /// impl PartialEq

    bool eq(const TupleHolder &) const { return true; }

    /// impl PartialOrd

    Option<Ordering> partial_cmp(const TupleHolder &) const {
        return make_some(Ordering::equal());
    }

    /// impl Ord

    Ordering cmp(const TupleHolder &) const { return Ordering::equal(); }
};

template<usize index, class ...Entries>
struct TupleGetter;

template<usize index, class Entry, class ...Entries>
struct TupleGetter<index, Entry, Entries...> {
    using Self = TupleHolder<Entry, Entries...>;
    using Result = typename TupleGetter<index - 1, Entries...>::Result;

    static constexpr const Result &inner(const Self &self) {
        return TupleGetter<index - 1, Entries...>::inner(self.remains);
    }

    static constexpr Result &inner(Self &self) {
        return TupleGetter<index - 1, Entries...>::inner(self.remains);
    }

    static constexpr const Result &&inner_move(const Self &&self) {
        return TupleGetter<index - 1, Entries...>::inner_move(move(self.remains));
    }

    static constexpr Result &&inner_move(Self &&self) {
        return TupleGetter<index - 1, Entries...>::inner_move(move(self.remains));
    }
};

template<class Entry, class ...Entries>
struct TupleGetter<0, Entry, Entries...> {
    using Self = TupleHolder<Entry, Entries...>;
    using Result = Entry;

    static constexpr const Result &inner(const Self &self) { return self.entry; }

    static constexpr Result &inner(Self &self) { return self.entry; }

    static constexpr const Result &&inner_move(const Self &&self) { return move(self.entry); }

    static constexpr Result &&inner_move(Self &&self) { return move(self.entry); }
};
}


template<class ...Entries>
struct Tuple : public Impl<
        PartialEq<Tuple<Entries...>>,
        CRUST_DERIVE(__impl::TupleHolder<Entries...>, PartialEq)
>, public Impl<
        Eq<Tuple<Entries...>>,
        CRUST_DERIVE(__impl::TupleHolder<Entries...>, Eq)
>, public Impl<
        PartialOrd<Tuple<Entries...>>,
        CRUST_DERIVE(__impl::TupleHolder<Entries...>, PartialOrd)
>, public Impl<
        Ord<Tuple<Entries...>>,
        CRUST_DERIVE(__impl::TupleHolder<Entries...>, Ord)
> {
private:
    using Holder = __impl::TupleHolder<typename RemoveReference<Entries>::Result...>;
    template<usize index> using Getter = __impl::TupleGetter<index, Entries...>;

    Holder holder;

public:
    template<usize index> using Result = typename Getter<index>::Result;

    template<class ...Ts>
    explicit constexpr Tuple(Ts &&...entries) noexcept: holder{forward<Ts>(entries)...} {}

    template<usize index>
    constexpr const Result<index> &get() const { return Getter<index>::inner(holder); }

    template<usize index>
    CRUST_CXX14_CONSTEXPR Result<index> &get() { return Getter<index>::inner(holder); }

    template<usize index>
    constexpr const Result<index> &&move() const {
        return Getter<index>::inner_move(move(holder));
    }

    template<usize index>
    CRUST_CXX14_CONSTEXPR Result<index> &&move() {
        return Getter<index>::inner_move(move(holder));
    }

    /// impl PartialEq

    bool eq(const Tuple &other) const { return this->holder == other.holder; }

    bool ne(const Tuple &other) const { return this->holder != other.holder; }

    /// impl PartialOrd

    Option<Ordering> partial_cmp(const Tuple &other) const {
        return this->holder.partial_cmp(other.holder);
    }

    bool lt(const Tuple &other) const { return this->holder < other.holder; }

    bool le(const Tuple &other) const { return this->holder <= other.holder; }

    bool gt(const Tuple &other) const { return this->holder > other.holder; }

    bool ge(const Tuple &other) const { return this->holder >= other.holder; }

    /// impl Ord

    Ordering cmp(const Tuple &other) const { return this->holder.cmp(other.holder); }
};

template<class ...Entries>
constexpr Tuple<Entries...> make_tuple(Entries &&...entries) {
    return Tuple<Entries...>(forward<Entries>(entries)...);
}
}


namespace std {

/// c++ std bindings

template<class ...Entries>
struct tuple_size<crust::Tuple<Entries...>> : integral_constant<crust::usize, sizeof...(Entries)> {
};

template<crust::usize index, class ...Entries>
struct tuple_element<index, crust::Tuple<Entries...>> {
    using type = typename crust::Tuple<Entries...>::template Result<index>;
};

template<crust::usize index, class ...Entries>
constexpr const class tuple_element<index, crust::Tuple<Entries...>>::type &
get(const crust::Tuple<Entries...> &object) noexcept {
    return object.template get<index>();
}

template<crust::usize index, class ...Entries>
constexpr typename tuple_element<index, crust::Tuple<Entries...>>::type &
get(crust::Tuple<Entries...> &object) noexcept {
    return object.template get<index>();
}

template<crust::usize index, class ...Entries>
constexpr const class tuple_element<index, crust::Tuple<Entries...>>::type &&
get(const crust::Tuple<Entries...> &&object) noexcept {
    return object.template move<index>();
}

template<crust::usize index, class ...Entries>
constexpr typename tuple_element<index, crust::Tuple<Entries...>>::type &&
get(crust::Tuple<Entries...> &&object) noexcept {
    return object.template move<index>();
}
}


#endif //CRUST_TUPLE_HPP
