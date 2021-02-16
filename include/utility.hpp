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

template<class T>
struct RemoveReference {
    using Result = T;
};

template<class T>
struct RemoveReference<T &> {
    using Result = T;
};

template<class T>
struct RemoveReference<T &&> {
    using Result = T;
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
constexpr typename RemoveReference<T>::Result &&move(T &&t) noexcept {
    return static_cast<typename RemoveReference<T>::Result &&>(t);
}

template<class T>
constexpr T &&forward(typename RemoveReference<T>::Result &t) noexcept {
    return static_cast<T &&>(t);
}

template<class T>
constexpr T &&forward(typename RemoveReference<T>::type &&t) noexcept {
    CRUST_STATIC_ASSERT(!IsLValueReference<T>::result);
    return static_cast<T &&>(t);
}

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

template<class T, bool condition>
struct __Impl;

template<class T>
struct __Impl<T, true> : public T {
};

template<class T>
struct __Impl<T, false> {
};

template<class T, bool ...conditions>
using Impl = __Impl<T, All<conditions...>::result>;

template<class Struct, class Trait>
struct Derive {
    static constexpr bool result = std::is_base_of<Trait, Struct>::value;
};

template<class Fn>
struct Function;

template<class __Type>
struct Function<void(__Type)> {
    using Type = __Type;
};

#define CRUST_DERIVE(Struct, Trait, ...) \
    ::crust::Derive<Struct, Trait<Struct, ##__VA_ARGS__>>::result

#define CRUST_DERIVE_PRIMITIVE(PRIMITIVE, TRAIT, ...) \
    template<> struct Derive<PRIMITIVE, TRAIT<PRIMITIVE, ##__VA_ARGS__>> { \
        static constexpr bool result = true; \
    }

#define CRUST_REQUIRE(Trait, ...) \
static_assert(CRUST_DERIVE(Self, Trait, ##__VA_ARGS__), \
"requires implementing trait `" #Trait "'!")

#define CRUST_TRAIT_DECLARE(TRAIT, ...) \
    template<class Self, ##__VA_ARGS__> \
    class TRAIT

#define CRUST_TRAIT(TRAIT, ...) \
    CRUST_TRAIT_DECLARE(TRAIT, ##__VA_ARGS__) { \
    public: \
        const Self &self() const { \
            CRUST_STATIC_ASSERT(std::is_base_of<TRAIT, Self>::value); \
            return *reinterpret_cast<const Self *>(this); \
        } \
        Self &self() { \
            CRUST_STATIC_ASSERT(std::is_base_of<TRAIT, Self>::value); \
            return *reinterpret_cast<Self *>(this); \
        }; \
    private:
}


#endif //CRUST_UTILITY_HPP
