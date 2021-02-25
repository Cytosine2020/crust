#ifndef CRUST_ENUM_DECLARE_HPP
#define CRUST_ENUM_DECLARE_HPP


#include <new>
#include <limits>

#include "utility.hpp"
#include "cmp_declare.hpp"
#include "tuple_declare.hpp"


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
struct EnumDuplicate;

template<class Field, class ...Fields>
struct EnumDuplicate<Field, Fields...> {
    static constexpr bool result = EnumInclude<Field, Fields...>::result ||
                                   EnumDuplicate<Fields...>::result;
};

template<>
struct EnumDuplicate<> {
    static constexpr bool result = false;
};


template<bool trivial, class ...Fields>
union EnumHolder;

template<class Field, class ...Fields>
union EnumHolder<true, Field, Fields...> {
    using Remains = EnumHolder<true, Fields...>;

    Field field;
    Remains remains;

    constexpr explicit EnumHolder(Field &&field) noexcept: field{move(field)} {}

    constexpr explicit EnumHolder(const Field &field) noexcept: field{field} {}

    template<class T>
    constexpr explicit EnumHolder(T &&field) noexcept: remains{forward<T>(field)} {
        CRUST_STATIC_ASSERT(!IsSame<typename RemoveConstRef<T>::Result, Field>::result);
    }
};

template<class Field, class ...Fields>
union EnumHolder<false, Field, Fields...> {
    using Remains = EnumHolder<false, Fields...>;

    Field field;
    Remains remains;

    constexpr EnumHolder() noexcept: remains{} {}

    constexpr explicit EnumHolder(Field &&field) noexcept: field{move(field)} {}

    constexpr explicit EnumHolder(const Field &field) noexcept: field{field} {}

    template<class T>
    constexpr explicit EnumHolder(T &&field) noexcept: remains{forward<T>(field)} {
        CRUST_STATIC_ASSERT(!IsSame<typename RemoveConstRef<T>::Result, Field>::result);
    }

    EnumHolder(const EnumHolder &) noexcept {}

    EnumHolder(EnumHolder &&) noexcept {}

    EnumHolder &operator=(const EnumHolder &) noexcept { return *this; }

    EnumHolder &operator=(EnumHolder &&) noexcept { return *this; }

    ~EnumHolder() {}
};

template<bool trivial>
union EnumHolder<trivial> {
};

template<usize index, class ...Fields>
struct EnumType;

template<usize index, class Field, class ...Fields>
struct EnumType<index, Field, Fields...> {
    using Result = typename EnumType<index - 1, Fields...>::Result;
};

template<class Field, class ...Fields>
struct EnumType<0, Field, Fields...> {
    using Result = Field;
};

template<usize index, bool trivial, class ...Fields>
struct EnumGetter;

template<usize index, bool trivial, class Field, class ...Fields>
struct EnumGetter<index, trivial, Field, Fields...> {
    using Self = EnumHolder<trivial, Field, Fields...>;
    using Remain = EnumGetter<index - 1, trivial, Fields...>;
    using Result = typename Remain::Result;

    static constexpr const Result &inner(const Self &self) { return Remain::inner(self.remains); }

    static constexpr Result &inner(Self &self) { return Remain::inner(self.remains); }
};

template<bool trivial, class Field, class ...Fields>
struct EnumGetter<0, trivial, Field, Fields...> {
    using Self = EnumHolder<trivial, Field, Fields...>;
    using Result = Field;

    static constexpr const Result &inner(const Self &self) { return self.field; }

    static constexpr Result &inner(Self &self) { return self.field; }

    static constexpr const Result &&inner_move(const Self &&self) { return move(self.field); }

    static constexpr Result &&inner_move(Self &&self) { return move(self.field); }
};

template<usize offset, usize size, bool trivial, class ...Fields>
struct EnumVisitor {
private:
    CRUST_STATIC_ASSERT(offset + size <= sizeof...(Fields));

    static constexpr usize cut = size / 2;

    using LowerGetter = EnumVisitor<offset, cut, trivial, Fields...>;
    using UpperGetter = EnumVisitor<offset + cut, size - cut, trivial, Fields...>;

public:
    template<class R, class V>
    static CRUST_CXX14_CONSTEXPR R
    inner(const EnumHolder<trivial, Fields...> &self, V &&impl, usize index) {
        return index < cut
               ? LowerGetter::template inner<R, V>(self, forward<V>(impl), index)
               : UpperGetter::template inner<R, V>(self, forward<V>(impl), index);
    }

    template<class R, class V>
    static CRUST_CXX14_CONSTEXPR R
    inner(EnumHolder<trivial, Fields...> &self, V &&impl, usize index) {
        return index < cut
               ? LowerGetter::template inner<R, V>(self, forward<V>(impl), index)
               : UpperGetter::template inner<R, V>(self, forward<V>(impl), index);
    }

    template<class R, class V>
    static CRUST_CXX14_CONSTEXPR R
    inner_move(const EnumHolder<trivial, Fields...> &&self, V &&impl, usize index) {
        return index < cut
               ? LowerGetter::template inner<R, V>(move(self), forward<V>(impl), index)
               : UpperGetter::template inner<R, V>(move(self), forward<V>(impl), index);
    }

    template<class R, class V>
    static CRUST_CXX14_CONSTEXPR R
    inner_move(EnumHolder<trivial, Fields...> &&self, V &&impl, usize index) {
        return index < cut
               ? LowerGetter::template inner<R, V>(move(self), forward<V>(impl), index)
               : UpperGetter::template inner<R, V>(move(self), forward<V>(impl), index);
    }
};

#define ENUM_VISITOR_BRANCH(index) \
    case offset + index: \
        return forward<V>(impl)(EnumGetter<offset + index, trivial, Fields...>::inner(self))

#define ENUM_VISITOR_IMPL(len) \
    template<usize offset, bool trivial, class ...Fields> \
    struct EnumVisitor<offset, len, trivial, Fields...> { \
        CRUST_STATIC_ASSERT(offset + len <= sizeof...(Fields)); \
        template<class R, class V> \
        static CRUST_CXX14_CONSTEXPR R \
        inner(const EnumHolder<trivial, Fields...> &self, V &&impl, usize index) { \
            switch (index) { \
                CRUST_DEFAULT_UNREACHABLE; \
                CRUST_MACRO_REPEAT(len, ENUM_VISITOR_BRANCH); \
            } \
        } \
        template<class R, class V> \
        static CRUST_CXX14_CONSTEXPR R \
        inner(EnumHolder<trivial, Fields...> &self, V &&impl, usize index) { \
            switch (index) { \
                CRUST_DEFAULT_UNREACHABLE; \
                CRUST_MACRO_REPEAT(len, ENUM_VISITOR_BRANCH); \
            } \
        } \
    }

ENUM_VISITOR_IMPL(16);

ENUM_VISITOR_IMPL(15);

ENUM_VISITOR_IMPL(14);

ENUM_VISITOR_IMPL(13);

ENUM_VISITOR_IMPL(12);

ENUM_VISITOR_IMPL(11);

ENUM_VISITOR_IMPL(10);

ENUM_VISITOR_IMPL(9);

ENUM_VISITOR_IMPL(8);

ENUM_VISITOR_IMPL(7);

ENUM_VISITOR_IMPL(6);

ENUM_VISITOR_IMPL(5);

ENUM_VISITOR_IMPL(4);

ENUM_VISITOR_IMPL(3);

ENUM_VISITOR_IMPL(2);

ENUM_VISITOR_IMPL(1);

#undef ENUM_VISITOR_IMPL
#undef ENUM_VISITOR_MOVE_BRANCH
#undef ENUM_VISITOR_BRANCH


template<usize offset, usize size, class ...Fields>
struct TagVisitor {
private:
    CRUST_STATIC_ASSERT(offset + size <= sizeof...(Fields));

    static constexpr usize cut = size / 2;

    using LowerGetter = TagVisitor<offset, cut, Fields...>;
    using UpperGetter = TagVisitor<offset + cut, size - cut, Fields...>;

public:
    template<class R, class V>
    static CRUST_CXX14_CONSTEXPR R inner(V &&impl, usize index) {
        return index < cut ? LowerGetter::template inner<R, V>(forward<V>(impl), index)
                           : UpperGetter::template inner<R, V>(forward<V>(impl), index);
    }

    template<class R, class V>
    static CRUST_CXX14_CONSTEXPR R inner_move(V &&impl, usize index) {
        return index < cut ? LowerGetter::template inner<R, V>(forward<V>(impl), index)
                           : UpperGetter::template inner<R, V>(forward<V>(impl), index);
    }
};

#define TAG_VISITOR_BRANCH(index) \
    case offset + index: { \
        auto tmp = typename EnumType<offset + index, Fields...>::Result{}; \
        return forward<V>(impl)(tmp); \
    }

#define TAG_VISITOR_IMPL(len) \
    template<usize offset, class ...Fields> \
    struct TagVisitor<offset, len, Fields...> { \
        CRUST_STATIC_ASSERT(offset + len <= sizeof...(Fields)); \
        template<class R, class V> \
        static CRUST_CXX14_CONSTEXPR R inner(V &&impl, usize index) { \
            switch (index) { \
                CRUST_DEFAULT_UNREACHABLE; \
                CRUST_MACRO_REPEAT(len, TAG_VISITOR_BRANCH); \
            } \
        } \
    }

TAG_VISITOR_IMPL(16);

TAG_VISITOR_IMPL(15);

TAG_VISITOR_IMPL(14);

TAG_VISITOR_IMPL(13);

TAG_VISITOR_IMPL(12);

TAG_VISITOR_IMPL(11);

TAG_VISITOR_IMPL(10);

TAG_VISITOR_IMPL(9);

TAG_VISITOR_IMPL(8);

TAG_VISITOR_IMPL(7);

TAG_VISITOR_IMPL(6);

TAG_VISITOR_IMPL(5);

TAG_VISITOR_IMPL(4);

TAG_VISITOR_IMPL(3);

TAG_VISITOR_IMPL(2);

TAG_VISITOR_IMPL(1);

#undef TAG_VISITOR_IMPL
#undef TAG_VISITOR_MOVE_BRANCH
#undef TAG_VISITOR_BRANCH


template<class T, class ...Fields>
struct EnumTypeToIndex;

template<class T, class Field, class ...Fields>
struct EnumTypeToIndex<T, Field, Fields...> {
    static constexpr u32 result = EnumTypeToIndex<T, Fields...>::result + 1;
};

template<class T, class ...Fields>
struct EnumTypeToIndex<T, T, Fields...> {
    static constexpr u32 result = 0;
};

template<class Self, bool flag>
struct EnumTrivial;

template<class Self>
struct EnumTrivial<Self, true> {
protected:
    void drop() {}
};

template<class Self>
struct EnumTrivial<Self, false> {
protected:
    CRUST_USE_SELF(EnumTrivial);

    void drop() { self().template visit<typename Self::__Drop, void>({}); };

public:
    constexpr EnumTrivial() noexcept = default;

    EnumTrivial(const EnumTrivial &other) noexcept {
        if (this != &other) {
            other.self().template visit<typename Self::__Copy, void>({&self().holder});
        }
    }

    EnumTrivial(EnumTrivial &&other) noexcept {
        if (this != &other) {
            other.self().template visit<typename Self::__Emplace, void>({&self().holder});
        }
    }

    EnumTrivial &operator=(const EnumTrivial &other) noexcept {
        if (this != &other) {
            drop();
            other.self().template visit<typename Self::__Copy, void>({&self().holder});
        }

        return *this;
    }

    EnumTrivial &operator=(EnumTrivial &&other) noexcept {
        if (this != &other) {
            drop();
            other.self().template visit<typename Self::__Emplace, void>({&self().holder});
        }

        return *this;
    }

    ~EnumTrivial() { drop(); }
};

template<class ...Fields>
struct EnumIsTrivial : public All<std::is_trivially_copyable<Fields>::value...> {
};

template<class ...Fields>
struct EnumIsTagOnly : public All<IsMonoState<Fields>::result...> {
};


template<class ...Fields>
struct CRUST_EBCO EnumTagUnion :
        public EnumTrivial<EnumTagUnion<Fields...>, EnumIsTrivial<Fields...>::result>,
        public cmp::PartialEq<EnumTagUnion<Fields...>>, public cmp::Eq<EnumTagUnion<Fields...>> {
    CRUST_STATIC_ASSERT(sizeof...(Fields) <= std::numeric_limits<u32>::max() &&
                        sizeof...(Fields) > 0 && !__impl_enum::EnumDuplicate<Fields...>::result);

    static constexpr bool __trivial = EnumIsTrivial<Fields...>::result;

    friend EnumTrivial<EnumTagUnion, __trivial>;

    using __Holder = EnumHolder<__trivial, typename RemoveRef<Fields>::Result...>;
    template<class T> using __IndexGetter = EnumTypeToIndex<T, Fields...>;
    using __Getter = EnumVisitor<0, sizeof...(Fields), __trivial, Fields...>;

    __Holder holder;
    u32 index;

    template<class T>
    constexpr EnumTagUnion(T &&value) noexcept:
            holder{forward<T>(value)},
            index{__IndexGetter<typename RemoveRef<T>::Result>::result} {
        CRUST_STATIC_ASSERT(!IsSame<RemoveRef<T>, EnumTagUnion>::result);
    }

    struct __Drop {
        template<class T>
        CRUST_CXX14_CONSTEXPR void operator()(T &value) const { value.~T(); }
    };

    struct __Copy {
        __Holder *holder;

        template<class T>
        void operator()(const T &value) { ::new(holder) __Holder{value}; }
    };

    struct __Emplace {
        __Holder *holder;

        template<class T>
        void operator()(T &value) { ::new(holder) __Holder{move(value)}; }
    };

    template<class T>
    EnumTagUnion &operator=(T &&value) noexcept {
        CRUST_STATIC_ASSERT(!IsSame<RemoveRef<T>, EnumTagUnion>::result);

        this->drop();

        ::new(&holder) __Holder{forward<T>(value)};
        index = __IndexGetter<typename RemoveRef<T>::Result>::result;

        return *this;
    }

    template<class V, class R>
    CRUST_CXX14_CONSTEXPR R visit(V &&visitor) const {
        CRUST_ASSERT(index < sizeof...(Fields));
        return __Getter::template inner<R, V>(holder, forward<V>(visitor), index);
    }

    template<class V, class R>
    CRUST_CXX14_CONSTEXPR R visit(V &&visitor) {
        CRUST_ASSERT(index < sizeof...(Fields));
        return __Getter::template inner<R, V>(holder, forward<V>(visitor), index);
    }

    template<class T, class ...Fs>
    CRUST_CXX14_CONSTEXPR bool let_helper(__impl_tuple::TupleHolder<Fs &...> ref) {
        constexpr usize i = __IndexGetter<typename RemoveRef<T>::Result>::result;

        if (index == i) {
            __impl_tuple::LetTupleHelper<sizeof...(Fs), Fs...>::inner(
                    ref, EnumGetter<i, __trivial, Fields...>::inner(holder));
            return true;
        } else {
            return false;
        }
    }

    /// impl PartialEq

    struct __Equal {
        const __Holder *other;

        template<class T>
        CRUST_CXX14_CONSTEXPR bool operator()(const T &value) const {
            constexpr usize i = __IndexGetter<typename RemoveRef<T>::Result>::result;
            return value == EnumGetter<i, __trivial, Fields...>::inner(*other);
        };
    };

    CRUST_CXX14_CONSTEXPR bool eq(const EnumTagUnion &other) const {
        return this->index == other.index && visit<__Equal, bool>({&other.holder});
    }

    struct __NotEqual {
        const __Holder *other;

        template<class T>
        CRUST_CXX14_CONSTEXPR bool operator()(const T &value) const {
            constexpr usize i = __IndexGetter<typename RemoveRef<T>::Result>::result;
            return value != EnumGetter<i, __trivial, Fields...>::inner(*other);
        };
    };

    CRUST_CXX14_CONSTEXPR bool ne(const EnumTagUnion &other) const {
        return this->index != other.index || visit<__NotEqual, bool>({&other.holder});
    }
};


template<class ...Fields>
struct CRUST_EBCO EnumTagOnly :
        public cmp::PartialEq<EnumTagOnly<Fields...>>,
        public cmp::Eq<EnumTagOnly<Fields...>> {
    template<class T> using __IndexGetter = EnumTypeToIndex<T, Fields...>;
    using __Getter = TagVisitor<0, sizeof...(Fields), Fields...>;

    u32 index;

    template<class T>
    constexpr EnumTagOnly(T &&) noexcept:
            index{__IndexGetter<typename RemoveRef<T>::Result>::result} {
        CRUST_STATIC_ASSERT(!IsSame<RemoveRef<T>, EnumTagOnly>::result);
    }

    template<class T>
    EnumTagOnly &operator=(T &&) noexcept {
        CRUST_STATIC_ASSERT(!IsSame<RemoveRef<T>, EnumTagOnly>::result);

        index = __IndexGetter<typename RemoveRef<T>::Result>::result;

        return *this;
    }

    template<class V, class R>
    CRUST_CXX14_CONSTEXPR R visit(V &&visitor) const {
        CRUST_ASSERT(index < sizeof...(Fields));
        return __Getter::template inner<R, V>(forward<V>(visitor), index);
    }

    template<class V, class R>
    CRUST_CXX14_CONSTEXPR R visit(V &&visitor) {
        CRUST_ASSERT(index < sizeof...(Fields));
        return __Getter::template inner<R, V>(forward<V>(visitor), index);
    }

    template<class T, class ...Fs>
    CRUST_CXX14_CONSTEXPR bool let_helper(__impl_tuple::TupleHolder<Fs &...> ref) {
        constexpr usize i = __IndexGetter<typename RemoveRef<T>::Result>::result;

        if (index == i) {
            __impl_tuple::LetTupleHelper<sizeof...(Fs), Fs...>(ref, T{});
            return true;
        } else {
            return false;
        }
    }

    constexpr bool eq(const EnumTagOnly &other) const {
        return this->index == other.index;
    }
};


template<bool is_tag_only, class ...Fields>
struct __EnumSelect;

template<class ...Fields>
struct __EnumSelect<false, Fields...> : public EnumTagUnion<Fields...> {
    CRUST_USE_BASE_CONSTRUCTORS(__EnumSelect, EnumTagUnion<Fields...>);
};

template<class ...Fields>
struct __EnumSelect<true, Fields...> : public EnumTagOnly<Fields...> {
    CRUST_USE_BASE_CONSTRUCTORS(__EnumSelect, EnumTagOnly<Fields...>);
};

template<class ...Fields>
using EnumSelect = __EnumSelect<EnumIsTagOnly<Fields...>::result, Fields...>;
}


template<class ...Fields>
class CRUST_EBCO Enum :
        public Impl<cmp::PartialEq<Enum<Fields...>>, CRUST_DERIVE(Fields, cmp::PartialEq)...>,
        public Impl<cmp::Eq<Enum<Fields...>>, CRUST_DERIVE(Fields, cmp::Eq)...> {
private:
    using Inner = __impl_enum::EnumSelect<Fields...>;

    Inner inner;

public:
    template<class T>
    constexpr Enum(T &&value) noexcept: inner{forward<T>(value)} {
        CRUST_STATIC_ASSERT(!IsSame<RemoveRef<T>, Enum>::result);
    }

    template<class T>
    Enum &operator=(T &&value) noexcept {
        CRUST_STATIC_ASSERT(!IsSame<RemoveRef<T>, Enum>::result);

        inner = forward<T>(value);
        return *this;
    }

    template<class V, class R = void>
    CRUST_CXX14_CONSTEXPR R visit(V &&visitor = V{}) const {
        return inner.template visit<V, R>(forward<V>(visitor));
    }

    template<class V, class R = void>
    CRUST_CXX14_CONSTEXPR R visit(V &&visitor = V{}) {
        return inner.template visit<V, R>(forward<V>(visitor));
    }

    template<class T, class ...Fs>
    CRUST_CXX14_CONSTEXPR bool let_helper(__impl_tuple::TupleHolder<Fs &...> ref) {
        return inner.template let_helper<T>(ref);
    }

    CRUST_CXX14_CONSTEXPR bool eq(const Enum &other) const { return inner == other.inner; }

    CRUST_CXX14_CONSTEXPR bool ne(const Enum &other) const { return inner != other.inner; }
};


namespace __impl_enum {
template<class T, class ...Fields>
struct LetEnum {
    __impl_tuple::TupleHolder<Fields &...> ref;

    explicit constexpr LetEnum(Fields &...ref) : ref{ref...} {}

    template<class ...Vs>
    CRUST_CXX14_CONSTEXPR bool operator=(Enum<Vs...> &&enum_) {
        return enum_.template let_helper<T>(ref);
    }
};
}


template<class T, class ...Fields>
CRUST_CXX14_CONSTEXPR __impl_enum::LetEnum<T, Fields...> let_enum(Fields &...fields) {
    return __impl_enum::LetEnum<T, Fields...>{fields...};
}
}


#endif //CRUST_ENUM_DECLARE_HPP
