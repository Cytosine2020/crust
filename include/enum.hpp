#ifndef CRUST_ENUM_DECLARE_HPP
#define CRUST_ENUM_DECLARE_HPP


#include <new>
#include <limits>

#include "utility.hpp"
#include "cmp_declare.hpp"


namespace crust {
namespace __impl_enum {
template<class T, class ...Fields>
struct EnumInclude;

template<class T, class Field, class ...Fields>
struct EnumInclude<T, Field, Fields...> {
    static constexpr bool result = EnumInclude<T, Fields...>::result;
};

template<class T, class ...Fields>
struct EnumInclude<T, T, Fields...> {
    static constexpr bool result = true;
};

template<class T>
struct EnumInclude<T> {
    static constexpr bool result = false;
};

template<class ...Fields>
union EnumHolder;

template<class Field, class ...Fields>
union EnumHolder<Field, Fields...> {
    Field field;
    EnumHolder<Fields...> remains;

    static constexpr bool dup = EnumInclude<Field, Fields...>::result ||
                                EnumHolder<Fields...>::dup;

    constexpr EnumHolder() noexcept: remains{} {}

    constexpr explicit EnumHolder(Field &&field) noexcept: field{move(field)} {}

    constexpr explicit EnumHolder(const Field &field) noexcept: field{field} {}

    template<class T>
    constexpr explicit EnumHolder(T &&field) noexcept: remains{forward<T>(field)} {
        CRUST_STATIC_ASSERT(!IsSame<typename RemoveConstRef<T>::Result, Field>::result);
    }

    ~EnumHolder() {}
};

template<>
union EnumHolder<> {
    static constexpr bool dup = false;
};

template<usize index, class ...Fields>
struct EnumGetter;

template<usize index, class Field, class ...Fields>
struct EnumGetter<index, Field, Fields...> {
    using Self = EnumHolder<Field, Fields...>;
    using Result = typename EnumGetter<index - 1, Fields...>::Result;

    static const Result &inner(const Self &self) {
        return EnumGetter<index - 1, Fields...>::inner(self.remains);
    }

    static Result &inner(Self &self) {
        return EnumGetter<index - 1, Fields...>::inner(self.remains);
    }

    static const Result &&inner_move(const Self &&self) {
        return EnumGetter<index - 1, Fields...>::inner_move(move(self.remains));
    }

    static Result &&inner_move(Self &&self) {
        return EnumGetter<index - 1, Fields...>::inner_move(move(self.remains));
    }
};

template<class Field, class ...Fields>
struct EnumGetter<0, Field, Fields...> {
    using Self = EnumHolder<Field, Fields...>;
    using Result = Field;

    static const Result &inner(const Self &self) { return self.field; }

    static Result &inner(Self &self) { return self.field; }

    static const Result &&inner_move(const Self &&self) { return move(self.field); }

    static Result &&inner_move(Self &&self) { return move(self.field); }
};

template<usize offset, usize size, class ...Fields>
struct EnumVisitor {
private:
    CRUST_STATIC_ASSERT(offset + size <= sizeof...(Fields));

    static constexpr usize cut = size / 2;

public:
    template<class R, class V>
    static R inner(const EnumHolder<Fields...> &self, V &&impl, usize index) {
        CRUST_ASSERT(index >= offset && index < offset + size);

        if (index < cut) {
            using Getter = EnumVisitor<offset, cut, Fields...>;
            return Getter::template inner<R, V>(self, forward<V>(impl), index);
        } else {
            using Getter = EnumVisitor<offset + cut, size - cut, Fields...>;
            return Getter::template inner<R, V>(self, forward<V>(impl), index);
        }
    }

    template<class R, class V>
    static R inner(EnumHolder<Fields...> &self, V &&impl, usize index) {
        CRUST_ASSERT(index >= offset && index < offset + size);

        if (index < cut) {
            using Getter = EnumVisitor<offset, cut, Fields...>;
            return Getter::template inner<R, V>(self, forward<V>(impl), index);
        } else {
            using Getter = EnumVisitor<offset + cut, size - cut, Fields...>;
            return Getter::template inner<R, V>(self, forward<V>(impl), index);
        }
    }

    template<class R, class V>
    static R inner_move(const EnumHolder<Fields...> &&self, V &&impl, usize index) {
        CRUST_ASSERT(index >= offset && index < offset + size);

        if (index < cut) {
            using Getter = EnumVisitor<offset, cut, Fields...>;
            return Getter::template inner_move<R, V>(move(self), forward<V>(impl), index);
        } else {
            using Getter = EnumVisitor<offset + cut, size - cut, Fields...>;
            return Getter::template inner_move<R, V>(move(self), forward<V>(impl), index);
        }
    }

    template<class R, class V>
    static R inner_move(EnumHolder<Fields...> &&self, V &&impl, usize index) {
        CRUST_ASSERT(index >= offset && index < offset + size);

        if (index < cut) {
            using Getter = EnumVisitor<offset, cut, Fields...>;
            return Getter::template inner_move<R, V>(move(self), forward<V>(impl), index);
        } else {
            using Getter = EnumVisitor<offset + cut, size - cut, Fields...>;
            return Getter::template inner_move<R, V>(move(self), forward<V>(impl), index);
        }
    }
};

#define VISITOR_BRANCH(index) \
    case offset + index: \
        return forward<V>(impl)(EnumGetter<offset + index, Fields...>::inner(self))

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
        return forward<V>(impl)(EnumGetter<offset + index, Fields...>::inner_move(move(self)))

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
    template<usize offset, class ...Fields> \
    struct EnumVisitor<offset, len, Fields...> { \
        CRUST_STATIC_ASSERT(offset + len <= sizeof...(Fields)); \
        template<class R, class V> \
        static R inner(const EnumHolder<Fields...> &self, V &&impl, usize index) { \
            switch (index) { \
                CRUST_DEFAULT_UNREACHABLE; \
                VISITOR_BRANCH_IMPL_##len; \
            } \
        } \
        template<class R, class V> \
        static R inner(EnumHolder<Fields...> &self, V &&impl, usize index) { \
            switch (index) { \
                CRUST_DEFAULT_UNREACHABLE; \
                VISITOR_BRANCH_IMPL_##len; \
            } \
        } \
        template<class R, class V> \
        static R inner_move(const EnumHolder<Fields...> &&self, V &&impl, usize index) { \
            switch (index) { \
                CRUST_DEFAULT_UNREACHABLE; \
                VISITOR_BRANCH_IMPL_MOVE_##len; \
            } \
        } \
        template<class R, class V> \
        static R inner_move(EnumHolder<Fields...> &&self, V &&impl, usize index) { \
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

template<class T, class ...Fields>
struct EnumTypeToIndex;

template<class T, class Field, class ...Fields>
struct EnumTypeToIndex<T, Field, Fields...> {
    static constexpr usize result = EnumTypeToIndex<T, Fields...>::result + 1;
};

template<class T, class ...Fields>
struct EnumTypeToIndex<T, T, Fields...> {
    static constexpr usize result = 0;
};
}


template<class ...Fields>
struct Enum :
        public Impl<PartialEq<Enum<Fields...>>, CRUST_DERIVE(Fields, PartialEq)...>,
        public Impl<Eq<Enum<Fields...>>, CRUST_DERIVE(Fields, Eq)...> {
private:
    using __Holder = __impl_enum::EnumHolder<typename RemoveRef<Fields>::Result...>;
    template<class T> using __IndexGetter = __impl_enum::EnumTypeToIndex<T, Fields...>;
    using __Getter = __impl_enum::EnumVisitor<0, sizeof...(Fields), Fields...>;

    CRUST_STATIC_ASSERT(sizeof...(Fields) < std::numeric_limits<u32>::max() && !__Holder::dup);

    __Holder holder;
    u32 index;

public:
    constexpr Enum() noexcept: holder{}, index{0} {}

    template<class T>
    constexpr Enum(T &&value) noexcept:
            holder{forward<T>(value)},
            index{__IndexGetter<typename RemoveRef<T>::Result>::result + 1} {}

    template<class V, class R = void>
    R visit(V &&visitor = V{}) const {
        if (index == 0) {
            crust_panic("Visiting a moved or destructed enum!");
        } else {
            return __Getter::template inner<R, V>(holder, forward<V>(visitor), index - 1);
        }
    }

    template<class V, class R = void>
    R visit(V &&visitor = V{}) {
        if (index == 0) {
            crust_panic("Visiting a moved or destructed enum!");
        } else {
            return __Getter::template inner<R, V>(holder, forward<V>(visitor), index - 1);
        }
    }

    template<class V, class R = void>
    R visit_move(V &&visitor = V{}) const {
        if (index == 0) {
            crust_panic("Visiting a moved or destructed enum!");
        } else {
            return __Getter::template inner_move<R, V>(move(holder), forward<V>(visitor), index - 1);
        }
    }

    template<class V, class R = void>
    R visit_move(V &&visitor = V{}) {
        if (index == 0) {
            crust_panic("Visiting a moved or destructed enum!");
        } else {
            return __Getter::template inner_move<R, V>(move(holder), forward<V>(visitor), index - 1);
        }
    }

private:
    struct __Drop {
        template<class T>
        void operator()(T &value) { value.~T(); }
    };

    void drop() {
        if (index != 0) {
            visit<__Drop>();
            index = 0;
        }
    };

    struct __Copy {
        __Holder *holder;

        template<class T>
        void operator()(const T &value) { ::new(holder) __Holder{value}; }
    };

public:
    Enum(const Enum &other) noexcept {
        if (this != &other) {
            index = other.index;
            if (other.index != 0) { other.visit<__Copy>({&holder}); }
        }
    }

    Enum &operator=(const Enum &other) noexcept {
        if (this != &other) {
            this->drop();

            index = other.index;
            if (other.index != 0) { other.visit<__Copy>({&holder}); }
        }

        return *this;
    }

private:
    struct __Emplace {
        __Holder *holder;

        template<class T>
        void operator()(T &&value) { ::new(holder) __Holder{forward<T>(value)}; }
    };

public:
    Enum(Enum &&other) noexcept {
        if (this != &other) {
            index = other.index;
            if (other.index != 0) { other.visit_move<__Emplace>({&holder}); }
            other.index = 0;
        }
    }

    Enum &operator=(Enum &&other) noexcept {
        if (this != &other) {
            this->drop();

            index = other.index;
            if (other.index != 0) { other.visit_move<__Emplace>({&holder}); }
            other.index = 0;
        }

        return *this;
    }

    template<class T>
    Enum &operator=(T &&value) noexcept {
        this->drop();

        ::new(&holder) __Holder{forward<T>(value)};
        index = __IndexGetter<typename RemoveRef<T>::Result>::result + 1;

        return *this;
    }

private:

    /// impl PartialEq

    struct __Equal {
        const __Holder *other;

        template<class T>
        bool operator()(const T &value) {
            constexpr usize i = __IndexGetter<typename RemoveRef<T>::Result>::result;
            return value == __impl_enum::EnumGetter<i, Fields...>::inner(*other);
        };
    };

public:
    bool eq(const Enum &other) const {
        if (this->index == 0 || other.index == 0) {
            crust_panic("Visiting a moved or destructed enum!");
        } else {
            return this->index == other.index && visit<__Equal, bool>({&other.holder});
        }
    }

private:
    struct __NotEqual {
        const __Holder *other;

        template<class T>
        bool operator()(const T &value) {
            constexpr usize i = __IndexGetter<typename RemoveRef<T>::Result>::result;
            return value != __impl_enum::EnumGetter<i, Fields...>::inner(*other);
        };
    };

public:
    bool ne(const Enum &other) const {
        if (this->index == 0 || other.index == 0) {
            crust_panic("Visiting a moved or destructed enum!");
        } else {
            return this->index != other.index || visit<__NotEqual, bool>({&other.holder});
        }
    }

    ~Enum() { drop(); }
};
}


#endif //CRUST_ENUM_DECLARE_HPP
