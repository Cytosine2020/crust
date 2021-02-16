#ifndef CRUST_ENUM_HPP
#define CRUST_ENUM_HPP


#include <new>
#include <limits>

#include "utility.hpp"


namespace crust {
namespace __impl {
template<class T, class ...Entries>
struct EnumInclude;

template<class T, class Entry, class ...Entries>
struct EnumInclude<T, Entry, Entries...> {
    static constexpr bool result = EnumInclude<T, Entries...>::result;
};

template<class T, class ...Entries>
struct EnumInclude<T, T, Entries...> {
    static constexpr bool result = true;
};

template<class T>
struct EnumInclude<T> {
    static constexpr bool result = false;
};

template<class ...Entries>
union EnumHolder;

template<class Entry, class ...Entries>
union EnumHolder<Entry, Entries...> {
    Entry entry;
    EnumHolder<Entries...> remains;

    static constexpr bool dup = EnumInclude<Entry, Entries...>::result ||
                                EnumHolder<Entries...>::dup;

    EnumHolder() noexcept: remains{} {}

    explicit EnumHolder(Entry &&entry) noexcept: entry{move(entry)} {}

    explicit EnumHolder(const Entry &&entry) noexcept: entry{move(entry)} {}

    explicit EnumHolder(Entry &entry) noexcept: entry{entry} {}

    explicit EnumHolder(const Entry &entry) noexcept: entry{entry} {}

    template<class T>
    explicit EnumHolder(T &&entry) noexcept: remains{forward<T>(entry)} {}

    ~EnumHolder() {}
};

template<>
union EnumHolder<> {
    static constexpr bool dup = false;
};

template<usize index, class ...Entries>
struct EnumGetter;

template<usize index, class Entry, class ...Entries>
struct EnumGetter<index, Entry, Entries...> {
    using Self = EnumHolder<Entry, Entries...>;
    using Result = typename EnumGetter<index - 1, Entries...>::Result;

    static const Result &inner(const Self &self) {
        return EnumGetter<index - 1, Entries...>::inner(self.remains);
    }

    static Result &inner(Self &self) {
        return EnumGetter<index - 1, Entries...>::inner(self.remains);
    }

    static const Result &&inner_move(const Self &&self) {
        return EnumGetter<index - 1, Entries...>::inner_move(move(self.remains));
    }

    static Result &&inner_move(Self &&self) {
        return EnumGetter<index - 1, Entries...>::inner_move(move(self.remains));
    }
};

template<class Entry, class ...Entries>
struct EnumGetter<0, Entry, Entries...> {
    using Self = EnumHolder<Entry, Entries...>;
    using Result = Entry;

    static const Result &inner(const Self &self) { return self.entry; }

    static Result &inner(Self &self) { return self.entry; }

    static const Result &&inner_move(const Self &&self) { return move(self.entry); }

    static Result &&inner_move(Self &&self) { return move(self.entry); }
};

template<usize offset, usize size, class ...Entries>
struct EnumVisitor {
private:
    CRUST_STATIC_ASSERT(offset + size <= sizeof...(Entries));

    static constexpr usize cut = size / 2;

public:
    template<class R, class V>
    static R inner(const EnumHolder<Entries...> &self, V &&impl, usize index) {
        CRUST_ASSERT(index >= offset && index < offset + size);

        if (index < cut) {
            using Getter = EnumVisitor<offset, cut, Entries...>;
            return Getter::template inner<R, V>(self, forward<V>(impl), index);
        } else {
            using Getter = EnumVisitor<offset + cut, size - cut, Entries...>;
            return Getter::template inner<R, V>(self, forward<V>(impl), index);
        }
    }

    template<class R, class V>
    static R inner(EnumHolder<Entries...> &self, V &&impl, usize index) {
        CRUST_ASSERT(index >= offset && index < offset + size);

        if (index < cut) {
            using Getter = EnumVisitor<offset, cut, Entries...>;
            return Getter::template inner<R, V>(self, forward<V>(impl), index);
        } else {
            using Getter = EnumVisitor<offset + cut, size - cut, Entries...>;
            return Getter::template inner<R, V>(self, forward<V>(impl), index);
        }
    }

    template<class R, class V>
    static R inner_move(const EnumHolder<Entries...> &&self, V &&impl, usize index) {
        CRUST_ASSERT(index >= offset && index < offset + size);

        if (index < cut) {
            using Getter = EnumVisitor<offset, cut, Entries...>;
            return Getter::template inner_move<R, V>(move(self), forward<V>(impl), index);
        } else {
            using Getter = EnumVisitor<offset + cut, size - cut, Entries...>;
            return Getter::template inner_move<R, V>(move(self), forward<V>(impl), index);
        }
    }

    template<class R, class V>
    static R inner_move(EnumHolder<Entries...> &&self, V &&impl, usize index) {
        CRUST_ASSERT(index >= offset && index < offset + size);

        if (index < cut) {
            using Getter = EnumVisitor<offset, cut, Entries...>;
            return Getter::template inner_move<R, V>(move(self), forward<V>(impl), index);
        } else {
            using Getter = EnumVisitor<offset + cut, size - cut, Entries...>;
            return Getter::template inner_move<R, V>(move(self), forward<V>(impl), index);
        }
    }
};

#define VISITOR_BRANCH(index) \
    case offset + index: \
        return forward<V>(impl)(EnumGetter<offset + index, Entries...>::inner(self))

#define VISITOR_BRANCH_IMPL_1 VISITOR_BRANCH(0)
#define VISITOR_BRANCH_IMPL_2 VISITOR_BRANCH_IMPL_1; VISITOR_BRANCH(1)
#define VISITOR_BRANCH_IMPL_3 VISITOR_BRANCH_IMPL_2; VISITOR_BRANCH(2)
#define VISITOR_BRANCH_IMPL_4 VISITOR_BRANCH_IMPL_3; VISITOR_BRANCH(3)
#define VISITOR_BRANCH_IMPL_5 VISITOR_BRANCH_IMPL_4; VISITOR_BRANCH(4)
#define VISITOR_BRANCH_IMPL_6 VISITOR_BRANCH_IMPL_5; VISITOR_BRANCH(5)
#define VISITOR_BRANCH_IMPL_7 VISITOR_BRANCH_IMPL_6; VISITOR_BRANCH(6)
#define VISITOR_BRANCH_IMPL_8 VISITOR_BRANCH_IMPL_7; VISITOR_BRANCH(7)
#define VISITOR_BRANCH_IMPL_9 VISITOR_BRANCH_IMPL_8; VISITOR_BRANCH(8)
#define VISITOR_BRANCH_IMPL_10 VISITOR_BRANCH_IMPL_9; VISITOR_BRANCH(9)
#define VISITOR_BRANCH_IMPL_11 VISITOR_BRANCH_IMPL_10; VISITOR_BRANCH(10)
#define VISITOR_BRANCH_IMPL_12 VISITOR_BRANCH_IMPL_11; VISITOR_BRANCH(11)
#define VISITOR_BRANCH_IMPL_13 VISITOR_BRANCH_IMPL_12; VISITOR_BRANCH(12)
#define VISITOR_BRANCH_IMPL_14 VISITOR_BRANCH_IMPL_13; VISITOR_BRANCH(13)
#define VISITOR_BRANCH_IMPL_15 VISITOR_BRANCH_IMPL_14; VISITOR_BRANCH(14)
#define VISITOR_BRANCH_IMPL_16 VISITOR_BRANCH_IMPL_15; VISITOR_BRANCH(15)

#define VISITOR_MOVE_BRANCH(index) \
    case offset + index: \
        return forward<V>(impl)(EnumGetter<offset + index, Entries...>::inner_move(move(self)))

#define VISITOR_BRANCH_IMPL_MOVE_1 VISITOR_MOVE_BRANCH(0)
#define VISITOR_BRANCH_IMPL_MOVE_2 VISITOR_BRANCH_IMPL_MOVE_1; VISITOR_MOVE_BRANCH(1)
#define VISITOR_BRANCH_IMPL_MOVE_3 VISITOR_BRANCH_IMPL_MOVE_2; VISITOR_MOVE_BRANCH(2)
#define VISITOR_BRANCH_IMPL_MOVE_4 VISITOR_BRANCH_IMPL_MOVE_3; VISITOR_MOVE_BRANCH(3)
#define VISITOR_BRANCH_IMPL_MOVE_5 VISITOR_BRANCH_IMPL_MOVE_4; VISITOR_MOVE_BRANCH(4)
#define VISITOR_BRANCH_IMPL_MOVE_6 VISITOR_BRANCH_IMPL_MOVE_5; VISITOR_MOVE_BRANCH(5)
#define VISITOR_BRANCH_IMPL_MOVE_7 VISITOR_BRANCH_IMPL_MOVE_6; VISITOR_MOVE_BRANCH(6)
#define VISITOR_BRANCH_IMPL_MOVE_8 VISITOR_BRANCH_IMPL_MOVE_7; VISITOR_MOVE_BRANCH(7)
#define VISITOR_BRANCH_IMPL_MOVE_9 VISITOR_BRANCH_IMPL_MOVE_8; VISITOR_MOVE_BRANCH(8)
#define VISITOR_BRANCH_IMPL_MOVE_10 VISITOR_BRANCH_IMPL_MOVE_9; VISITOR_MOVE_BRANCH(9)
#define VISITOR_BRANCH_IMPL_MOVE_11 VISITOR_BRANCH_IMPL_MOVE_10; VISITOR_MOVE_BRANCH(10)
#define VISITOR_BRANCH_IMPL_MOVE_12 VISITOR_BRANCH_IMPL_MOVE_11; VISITOR_MOVE_BRANCH(11)
#define VISITOR_BRANCH_IMPL_MOVE_13 VISITOR_BRANCH_IMPL_MOVE_12; VISITOR_MOVE_BRANCH(12)
#define VISITOR_BRANCH_IMPL_MOVE_14 VISITOR_BRANCH_IMPL_MOVE_13; VISITOR_MOVE_BRANCH(13)
#define VISITOR_BRANCH_IMPL_MOVE_15 VISITOR_BRANCH_IMPL_MOVE_14; VISITOR_MOVE_BRANCH(14)
#define VISITOR_BRANCH_IMPL_MOVE_16 VISITOR_BRANCH_IMPL_MOVE_15; VISITOR_MOVE_BRANCH(15)

#define VISITOR_IMPL(len) \
    template<usize offset, class ...Entries> \
    struct EnumVisitor<offset, len, Entries...> { \
        CRUST_STATIC_ASSERT(offset + len <= sizeof...(Entries)); \
        template<class R, class V> \
        static R inner(const EnumHolder<Entries...> &self, V &&impl, usize index) { \
            switch (index) { \
                CRUST_DEFAULT_UNREACHABLE; \
                VISITOR_BRANCH_IMPL_##len; \
            } \
        } \
        template<class R, class V> \
        static R inner(EnumHolder<Entries...> &self, V &&impl, usize index) { \
            switch (index) { \
                CRUST_DEFAULT_UNREACHABLE; \
                VISITOR_BRANCH_IMPL_##len; \
            } \
        } \
        template<class R, class V> \
        static R inner_move(const EnumHolder<Entries...> &&self, V &&impl, usize index) { \
            switch (index) { \
                CRUST_DEFAULT_UNREACHABLE; \
                VISITOR_BRANCH_IMPL_MOVE_##len; \
            } \
        } \
        template<class R, class V> \
        static R inner_move(EnumHolder<Entries...> &&self, V &&impl, usize index) { \
            switch (index) { \
                CRUST_DEFAULT_UNREACHABLE; \
                VISITOR_BRANCH_IMPL_MOVE_##len; \
            } \
        } \
    }

VISITOR_IMPL(16);

VISITOR_IMPL(15);

VISITOR_IMPL(14);

VISITOR_IMPL(13);

VISITOR_IMPL(12);

VISITOR_IMPL(11);

VISITOR_IMPL(10);

VISITOR_IMPL(9);

VISITOR_IMPL(8);

VISITOR_IMPL(7);

VISITOR_IMPL(6);

VISITOR_IMPL(5);

VISITOR_IMPL(4);

VISITOR_IMPL(3);

VISITOR_IMPL(2);

VISITOR_IMPL(1);

#undef VISITOR_IMPL
#undef VISITOR_BRANCH_IMPL_MOVE_16
#undef VISITOR_BRANCH_IMPL_MOVE_15
#undef VISITOR_BRANCH_IMPL_MOVE_14
#undef VISITOR_BRANCH_IMPL_MOVE_13
#undef VISITOR_BRANCH_IMPL_MOVE_12
#undef VISITOR_BRANCH_IMPL_MOVE_11
#undef VISITOR_BRANCH_IMPL_MOVE_10
#undef VISITOR_BRANCH_IMPL_MOVE_9
#undef VISITOR_BRANCH_IMPL_MOVE_8
#undef VISITOR_BRANCH_IMPL_MOVE_7
#undef VISITOR_BRANCH_IMPL_MOVE_6
#undef VISITOR_BRANCH_IMPL_MOVE_5
#undef VISITOR_BRANCH_IMPL_MOVE_4
#undef VISITOR_BRANCH_IMPL_MOVE_3
#undef VISITOR_BRANCH_IMPL_MOVE_2
#undef VISITOR_BRANCH_IMPL_MOVE_1
#undef VISITOR_MOVE_BRANCH
#undef VISITOR_BRANCH_IMPL_16
#undef VISITOR_BRANCH_IMPL_15
#undef VISITOR_BRANCH_IMPL_14
#undef VISITOR_BRANCH_IMPL_13
#undef VISITOR_BRANCH_IMPL_12
#undef VISITOR_BRANCH_IMPL_11
#undef VISITOR_BRANCH_IMPL_10
#undef VISITOR_BRANCH_IMPL_9
#undef VISITOR_BRANCH_IMPL_8
#undef VISITOR_BRANCH_IMPL_7
#undef VISITOR_BRANCH_IMPL_6
#undef VISITOR_BRANCH_IMPL_5
#undef VISITOR_BRANCH_IMPL_4
#undef VISITOR_BRANCH_IMPL_3
#undef VISITOR_BRANCH_IMPL_2
#undef VISITOR_BRANCH_IMPL_1
#undef VISITOR_BRANCH

template<class T, class ...Entries>
struct EnumTypeToIndex;

template<class T, class Entry, class ...Entries>
struct EnumTypeToIndex<T, Entry, Entries...> {
    static constexpr usize result = EnumTypeToIndex<T, Entries...>::result + 1;
};

template<class T, class ...Entries>
struct EnumTypeToIndex<T, T, Entries...> {
    static constexpr usize result = 0;
};
}


CRUST_TRAIT_DECLARE(PartialEq, class Rhs);

CRUST_TRAIT_DECLARE(Eq, class Rhs);


template<class ...Entries>
struct Enum : public Impl<
        PartialEq<Enum<Entries...>, Enum<Entries...>>,
        CRUST_DERIVE(Entries, PartialEq, Entries)...
>, public Impl<
        Eq<Enum<Entries...>, Enum<Entries...>>,
        CRUST_DERIVE(Entries, Eq, Entries)...
> {
private:
    using Holder = __impl::EnumHolder<typename RemoveReference<Entries>::Result...>;
    template<class T> using IndexGetter = __impl::EnumTypeToIndex<T, Entries...>;
    using Getter = __impl::EnumVisitor<0, sizeof...(Entries), Entries...>;

    CRUST_STATIC_ASSERT(sizeof...(Entries) < std::numeric_limits<u32>::max() && !Holder::dup);

    Holder holder;
    u32 index;

public:
    Enum() noexcept: holder{}, index{0} {}

    template<class T>
    explicit Enum(T &&value) noexcept:
            holder{forward<T>(value)},
            index{IndexGetter<typename RemoveReference<T>::Result>::result + 1} {}

    template<class R, class V>
    R visit(V &&visitor) const {
        if (index == 0) {
            crust_panic("Visiting a moved or destructed enum!");
        } else {
            return Getter::template inner<R, V>(holder, forward<V>(visitor), index - 1);
        }
    }

    template<class R, class V>
    R visit(V &&visitor) {
        if (index == 0) {
            crust_panic("Visiting a moved or destructed enum!");
        } else {
            return Getter::template inner<R, V>(holder, forward<V>(visitor), index - 1);
        }
    }

private:
    template<class R, class V>
    R visit_move(V &&visitor) const {
        if (index == 0) {
            crust_panic("Visiting a moved or destructed enum!");
        } else {
            return Getter::template inner_move<R, V>(move(holder), forward<V>(visitor), index - 1);
        }
    }

    template<class R, class V>
    R visit_move(V &&visitor) {
        if (index == 0) {
            crust_panic("Visiting a moved or destructed enum!");
        } else {
            return Getter::template inner_move<R, V>(move(holder), forward<V>(visitor), index - 1);
        }
    }

    struct Drop {
        template<class T>
        void operator()(T &value) { value.~T(); }
    };

    void drop() {
        if (index != 0) {
            visit<void>(Drop{});
            index = 0;
        }
    };

    struct Emplace {
        Holder *holder;

        template<class T>
        void operator()(T &&value) { ::new(holder) Holder{forward<T>(value)}; }
    };

public:
    Enum(Enum &&other) noexcept {
        if (this != &other) {
            index = other.index;
            if (other.index != 0) { other.visit_move<void>(Emplace{&holder}); }
            other.index = 0;
        }
    }

    Enum &operator=(Enum &&other) noexcept {
        if (this != &other) {
            this->drop();

            index = other.index;
            if (other.index != 0) { other.visit_move<void>(Emplace{&holder}); }
            other.index = 0;
        }

        return *this;
    }

private:

    /// impl PartialEq

    struct Equal {
        const Holder *other;

        template<class T>
        bool operator()(const T &value) {
            constexpr usize i = IndexGetter<typename RemoveReference<T>::Result>::result;
            return value == __impl::EnumGetter<i, Entries...>::inner(*other);
        };
    };

public:
    bool eq(const Enum &other) const {
        if (this->index == 0 || other.index == 0) {
            crust_panic("Visiting a moved or destructed enum!");
        } else {
            return this->index == other.index && visit<bool>(Equal{&other.holder});
        }
    }

private:
    struct NotEqual {
        const Holder *other;

        template<class T>
        bool operator()(const T &value) {
            constexpr usize i = IndexGetter<typename RemoveReference<T>::Result>::result;
            return value != __impl::EnumGetter<i, Entries...>::inner(*other);
        };
    };

public:
    bool ne(const Enum &other) const {
        if (this->index == 0 || other.index == 0) {
            crust_panic("Visiting a moved or destructed enum!");
        } else {
            return this->index != other.index || visit<bool>(NotEqual{&other.holder});
        }
    }

    ~Enum() { drop(); }
};
}


#endif //CRUST_ENUM_HPP
