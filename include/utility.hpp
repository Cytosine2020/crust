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

static inline void _assert(const char *file, int line, const char *msg, bool condition) {
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
struct IsLValueReference {
    static constexpr bool result = false;
};

template<class T>
struct IsLValueReference<T &> {
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
    CRUST_STATIC_ASSERT(!IsLValueReference<T>::result);
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

template<class Struct, class Trait>
struct Derive {
    static constexpr bool result = std::is_base_of<Trait, Struct>::value;
};

struct MonoStateTag {
};

template<class T>
struct IsMonoState {
    static constexpr bool result = std::is_base_of<MonoStateTag, T>::value;
};

#define CRUST_ECHO(...) __VA_ARGS__

#define CRUST_DERIVE(Struct, Trait, ...) \
    ::crust::Derive<Struct, Trait<Struct, ##__VA_ARGS__>>::result

#define CRUST_DERIVE_PRIMITIVE(PRIMITIVE, TRAIT, ...) \
    template<> struct Derive<PRIMITIVE, TRAIT<PRIMITIVE, ##__VA_ARGS__>> { \
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
    template<class __T = __VA_ARGS__> \
    decltype(static_cast<const __T *>(nullptr)->template \
    __detect_trait_partial_eq<>(*static_cast<const __T *>(nullptr))) \
    __detect_trait_partial_eq(const NAME &) const {} \
    template<class __T = __VA_ARGS__> \
    decltype(static_cast<const __T *>(nullptr)->template \
    __detect_trait_eq<>(*static_cast<const __T *>(nullptr))) \
    __detect_trait_eq(const NAME &) const {}

#define CRUST_ENUM_USE_BASE(NAME, ...) \
    CRUST_USE_BASE_CONSTRUCTORS(NAME, __VA_ARGS__) \
    CRUST_USE_BASE_TRAIT_EQ(NAME, __VA_ARGS__)

#define CRUST_ENUM_VARIANTS(NAME, ...) \
    struct NAME final : public Tuple<__VA_ARGS__> { \
        CRUST_USE_BASE_CONSTRUCTORS(NAME, Tuple<__VA_ARGS__>) \
        CRUST_USE_BASE_TRAIT_EQ(NAME, Tuple<__VA_ARGS__>) \
    }
}


#endif //CRUST_UTILITY_HPP
