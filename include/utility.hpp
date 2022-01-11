#ifndef CRUST_UTILITY_HPP
#define CRUST_UTILITY_HPP


#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <type_traits>


namespace crust {
#if __cplusplus > 201103L
#define CRUST_CXX14_CONSTEXPR constexpr
#else
#define CRUST_CXX14_CONSTEXPR
#endif

#define CRUST_STATIC_ASSERT(...) static_assert(__VA_ARGS__, #__VA_ARGS__)

#if defined(__GNUC__) || defined(__clang__)
#define CRUST_DEFAULT_UNREACHABLE default: __builtin_unreachable()
#elif defined(_MSC_VER)
#define CRUST_DEFAULT_UNREACHABLE default: __assume(false)
#else
#define CRUST_DEFAULT_UNREACHABLE
#endif

#if defined(_MSC_VER)
#define CRUST_EBCO __declspec(empty_bases)
#else
#define CRUST_EBCO
#endif

#if INTPTR_MAX == INT32_MAX
#define CRUST_BIT_WIDTH 32
#elif INTPTR_MAX == INT64_MAX
#define CRUST_BIT_WIDTH 64
#else
#error "Unsupported bit width."
#endif


[[noreturn]] static inline void __panic(const char *file, int line, const char *msg) {
    fprintf(stderr, "Abort at file %s, line %d: %s\n", file, line, msg);

    abort();
}

#define CRUST_PANIC(msg) ::crust::__panic(__FILE__, __LINE__, msg)

static inline CRUST_CXX14_CONSTEXPR void
_assert(const char *file, int line, const char *msg, bool condition) {
    if (!condition) {
        __panic(file, line, msg);
    }
}

#define CRUST_ASSERT(...) ::crust::_assert(__FILE__, __LINE__, #__VA_ARGS__, __VA_ARGS__)

using i8 = int8_t;
using u8 = uint8_t;
using i16 = int16_t;
using u16 = uint16_t;
using i32 = int32_t;
using u32 = uint32_t;
using i64 = int64_t;
using u64 = uint64_t;
#if CRUST_BIT_WIDTH == 32
using isize = i32;
using usize = u32;
#elif CRUST_BIT_WIDTH == 64
using isize = i64;
using usize = u64;
#else
#error "Unsupported bit width."
#endif


template<class A, class B>
struct IsSame {
    static constexpr bool result = false;
};

template<class A>
struct IsSame<A, A> {
    static constexpr bool result = true;
};

template<bool ...conditions>
struct All;

template<bool condition, bool ...conditions>
struct All<condition, conditions...> {
    static constexpr bool result = condition && All<conditions...>::result;
};

template<>
struct All<> {
    static constexpr bool result = true;
};

template<class T>
struct RemoveRef {
    using Result = T;
};

template<class T>
struct RemoveRef<T &> {
    using Result = T;
};

template<class T>
struct RemoveRef<T &&> {
    using Result = T;
};

template<class T>
struct RemoveConst {
    using Result = T;
};

template<class T>
struct RemoveConst<const T> {
    using Result = T;
};

template<class T>
struct RemoveConstRef {
    using Result = typename RemoveRef<typename RemoveConst<T>::Result>::Result;
};

template<class T>
struct IsLValueRef {
    static constexpr bool result = false;
};

template<class T>
struct IsLValueRef<T &> {
    static constexpr bool result = true;
};

template<class T>
constexpr typename RemoveRef<T>::Result &&move(T &&t) noexcept {
    return static_cast<typename RemoveRef<T>::Result &&>(t);
}

template<class T>
constexpr T &&forward(typename RemoveRef<T>::Result &t) noexcept {
    return static_cast<T &&>(t);
}

template<class T>
constexpr T &&forward(typename RemoveRef<T>::type &&t) noexcept {
    CRUST_STATIC_ASSERT(!IsLValueRef<T>::result);
    return static_cast<T &&>(t);
}

namespace __impl_impl {
template<class T, bool condition>
struct Impl;

template<class T>
struct Impl<T, true> : public T {
};

template<class T>
struct Impl<T, false> {
};
}


template<class T, bool ...conditions>
using Impl = __impl_impl::Impl<T, All<conditions...>::result>;

template<class Struct, template<class Self, class ...Args> class Trait, class ...Args>
struct Derive {
    static constexpr bool result = std::is_base_of<Trait<Struct, Args...>, Struct>::value;
};

/// this tag is used for enum optimization

struct MonoStateTag {
};

template<class T>
struct IsMonoState {
    static constexpr bool result = std::is_base_of<MonoStateTag, T>::value;
};

/// this tag is used for enum optimization

struct TransparentTag {
};

template<class T>
struct IsTransparent {
    static constexpr bool result = std::is_base_of<TransparentTag, T>::value;
};


#define CRUST_DERIVE_PRIMITIVE(PRIMITIVE, TRAIT, ...) \
    struct Derive<PRIMITIVE, TRAIT, ##__VA_ARGS__> { \
        static constexpr bool result = true; \
    }

#define CRUST_TRAIT_DECLARE(TRAIT, ...) \
    template<class Self, ##__VA_ARGS__> \
    class TRAIT

#define CRUST_USE_SELF(TRAIT) \
    constexpr const Self &self() const { \
        CRUST_STATIC_ASSERT(std::is_base_of<TRAIT, Self>::value); \
        return *static_cast<const Self *>(this); \
    } \
    CRUST_CXX14_CONSTEXPR Self &self() { \
        CRUST_STATIC_ASSERT(std::is_base_of<TRAIT, Self>::value); \
        return *static_cast<Self *>(this); \
    }

#define CRUST_TRAIT(TRAIT, ...) \
    CRUST_TRAIT_DECLARE(TRAIT, ##__VA_ARGS__) { \
    public: \
        CRUST_USE_SELF(TRAIT); \
    private:

#define CRUST_USE_BASE_CONSTRUCTORS(NAME, ...) \
    template<class ...Args> \
    constexpr NAME(Args &&...args) : __VA_ARGS__{::crust::forward<Args>(args)...} {}

#define CRUST_USE_BASE_CONSTRUCTORS_EXPLICIT(NAME, ...) \
    template<class ...Args> \
    explicit constexpr NAME(Args &&...args) : __VA_ARGS__{::crust::forward<Args>(args)...} {}

#define CRUST_USE_BASE_TRAIT_EQ(NAME, ...) \
    template<class = void> \
    static void __detect_trait_partial_eq(const NAME &) { \
        CRUST_STATIC_ASSERT(Derive<__VA_ARGS__, ::crust::cmp::PartialEq>); \
    } \
    template<class = void> \
    static void __detect_trait_eq() { \
        CRUST_STATIC_ASSERT(Derive<__VA_ARGS__, ::crust::cmp::Eq>); \
    }

#define CRUST_ENUM_USE_BASE(NAME, ...) \
    CRUST_USE_BASE_CONSTRUCTORS(NAME, __VA_ARGS__) \
    CRUST_USE_BASE_TRAIT_EQ(NAME, __VA_ARGS__)

#define CRUST_ENUM_VARIANT(NAME, ...) \
    struct NAME final : public Tuple<__VA_ARGS__> { \
        using Inner = Tuple<__VA_ARGS__>; \
        CRUST_USE_BASE_CONSTRUCTORS(NAME, Tuple<__VA_ARGS__>) \
        CRUST_USE_BASE_TRAIT_EQ(NAME, Tuple<__VA_ARGS__>) \
    }

#define CRUST_MACRO_REPEAT_1(FN) FN(0)
#define CRUST_MACRO_REPEAT_2(FN) CRUST_MACRO_REPEAT_1(FN); FN(1)
#define CRUST_MACRO_REPEAT_3(FN) CRUST_MACRO_REPEAT_2(FN); FN(2)
#define CRUST_MACRO_REPEAT_4(FN) CRUST_MACRO_REPEAT_3(FN); FN(3)
#define CRUST_MACRO_REPEAT_5(FN) CRUST_MACRO_REPEAT_4(FN); FN(4)
#define CRUST_MACRO_REPEAT_6(FN) CRUST_MACRO_REPEAT_5(FN); FN(5)
#define CRUST_MACRO_REPEAT_7(FN) CRUST_MACRO_REPEAT_6(FN); FN(6)
#define CRUST_MACRO_REPEAT_8(FN) CRUST_MACRO_REPEAT_7(FN); FN(7)
#define CRUST_MACRO_REPEAT_9(FN) CRUST_MACRO_REPEAT_8(FN); FN(8)
#define CRUST_MACRO_REPEAT_10(FN) CRUST_MACRO_REPEAT_9(FN); FN(9)
#define CRUST_MACRO_REPEAT_11(FN) CRUST_MACRO_REPEAT_10(FN); FN(10)
#define CRUST_MACRO_REPEAT_12(FN) CRUST_MACRO_REPEAT_11(FN); FN(11)
#define CRUST_MACRO_REPEAT_13(FN) CRUST_MACRO_REPEAT_12(FN); FN(12)
#define CRUST_MACRO_REPEAT_14(FN) CRUST_MACRO_REPEAT_13(FN); FN(13)
#define CRUST_MACRO_REPEAT_15(FN) CRUST_MACRO_REPEAT_14(FN); FN(14)
#define CRUST_MACRO_REPEAT_16(FN) CRUST_MACRO_REPEAT_15(FN); FN(15)
#define CRUST_MACRO_REPEAT(N, FN) CRUST_MACRO_REPEAT_##N(FN)
}


#endif //CRUST_UTILITY_HPP
