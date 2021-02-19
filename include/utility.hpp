#ifndef CRUST_UTILITY_HPP
#define CRUST_UTILITY_HPP


#include <cstdio>
#include <cstdlib>
#include <cstddef>
#include <type_traits>


#include <execinfo.h>


namespace crust {
#if __cplusplus > 201103L
#define CRUST_CXX14_CONSTEXPR constexpr
#else
#define CRUST_CXX14_CONSTEXPR
#endif

#define CRUST_STATIC_ASSERT(...) static_assert(__VA_ARGS__, #__VA_ARGS__)

#define CRUST_UNREACHABLE __builtin_unreachable()

#define CRUST_DEFAULT_UNREACHABLE default: CRUST_UNREACHABLE

#define crust_static_inline static inline __attribute__((always_inline))
#define crust_unused __attribute__((unused))
#define crust_no_return __attribute__((noreturn))
#if defined(__DEBUG__)
#define crust_inline inline
#else
#define crust_inline inline __attribute__((always_inline))
#endif

crust_static_inline void _warn(const char *file, int line, const char *msg) {
    fprintf(stderr, "Warn at file %s, line %d: %s\n", file, line, msg);
}

#define crust_warn(msg) ::crust::_warn(__FILE__, __LINE__, msg)

crust_static_inline crust_no_return void _abort(const char *file, int line, const char *msg) {
    fprintf(stderr, "Abort at file %s, line %d: %s\n", file, line, msg);

    void *callstack[128];
    int i, frames = backtrace(callstack, 128);
    char **strs = backtrace_symbols(callstack, frames);
    for (i = 0; i < frames; ++i) { fprintf(stderr, "%s\n", strs[i]); }
    free(strs);

    abort();
}

#define crust_panic(msg) ::crust::_abort(__FILE__, __LINE__, msg)

void _assert(const char *file, int line, const char *msg, bool condition) {
    if (!condition) {
        _abort(file, line, msg);
    }
}

#define CRUST_ASSERT(...) ::crust::_assert(__FILE__, __LINE__, #__VA_ARGS__, __VA_ARGS__)

using i8 = int8_t;
using u8 = u_int8_t;
using i16 = int16_t;
using u16 = u_int16_t;
using i32 = int32_t;
using u32 = u_int32_t;
using i64 = int64_t;
using u64 = u_int64_t;
using i128 = __int128;
using u128 = unsigned __int128;
using isize = ssize_t;
using usize = size_t;

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

namespace __impl_type {
template<class Fn>
struct ExtractType;

template<class Type>
struct ExtractType<void(Type)> {
    using Result = Type;
};
}


#define CRUST_DERIVE(Struct, Trait, ...) \
    ::crust::Derive<typename ::crust::__impl_type::ExtractType<void(Struct)>::Result, Trait< \
            typename ::crust::__impl_type::ExtractType<void(Struct)>::Result, ##__VA_ARGS__> \
    >::result

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

#define CRUST_USE_BASE_EQ(SELF, BASE) \
    void __detect_trait_partial_eq(const SELF &other) const { \
        using Base = typename ::crust::__impl_type::ExtractType<void(BASE)>::Result; \
        static_cast<const Base *>(this)->__detect_trait_partial_eq(other); \
    } \
    void __detect_trait_eq(const SELF &other) const { \
        using Base = typename ::crust::__impl_type::ExtractType<void(BASE)>::Result; \
        static_cast<const Base *>(this)->__detect_trait_eq(other); \
    }

#define CRUST_USE_BASE_CONSTRUCTOR(SELF, BASE) \
    template<class ...Args> \
    constexpr SELF(Args ...args) :             \
            ::crust::__impl_type::ExtractType<void(BASE)>::Result{ \
                    ::crust::forward<Args>(args)... \
    } {}


#define CRUST_ENUM_USE_BASE(SELF, BASE) \
    CRUST_USE_BASE_CONSTRUCTOR(SELF, BASE); \
    CRUST_USE_BASE_EQ(SELF, BASE)

#define CRUST_USE_BASE_CONSTRUCTOR_EXPLICIT(SELF, BASE) \
    template<class ...Args> \
    explicit constexpr SELF(Args ...args) :             \
            ::crust::__impl_type::ExtractType<void(BASE)>::Result{ \
                    ::crust::forward<Args>(args)... \
    } {}

#define CRUST_ENUM_FIELD(NAME, ...) \
    struct NAME : \
            public Tuple<__VA_ARGS__> { \
        CRUST_USE_BASE_CONSTRUCTOR_EXPLICIT(NAME, (Tuple<__VA_ARGS__>)); \
        CRUST_USE_BASE_EQ(NAME, (Tuple<__VA_ARGS__>)); \
    }
}


#endif //CRUST_UTILITY_HPP
