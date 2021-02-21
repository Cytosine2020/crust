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

#if defined(__GNUC__) || defined(__clang__)
#define CRUST_FUNCTION __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#define CRUST_FUNCTION __FUNCSIG__
#else
#define CRUST_FUNCTION __FUNCTION__
#endif

#define crust_static_inline static inline

crust_static_inline void _warn(const char *file, int line, const char *msg) {
    fprintf(stderr, "Warn at file %s, line %d: %s\n", file, line, msg);
}

#define crust_warn(msg) ::crust::_warn(__FILE__, __LINE__, msg)

[[noreturn]] crust_static_inline void _abort(const char *file, int line, const char *msg) {
    fprintf(stderr, "Abort at file %s, line %d: %s\n", file, line, msg);

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
using u8 = uint8_t;
using i16 = int16_t;
using u16 = uint16_t;
using i32 = int32_t;
using u32 = uint32_t;
using i64 = int64_t;
using u64 = uint64_t;
#if defined(__GNUC__) || defined(__clang__)
using isize = ssize_t;
#elif defined(_MSC_VER)
#include <BaseTsd.h>
using isize = SSIZE_T;
#else
using isize = ssize_t;
#endif
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

#define CRUST_ECHO(...) __VA_ARGS__

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

#define CRUST_ENUM_USE_BASE(NAME, ...) \
    template<class ...Args>     \
    constexpr NAME(Args ...args) : Enum<__VA_ARGS__>{::crust::forward<Args>(args)...} {} \
    void __detect_trait_partial_eq(const NAME &other) const { \
        static_cast<const Enum<__VA_ARGS__> *>(this)->__detect_trait_partial_eq(other); \
    } \
    void __detect_trait_eq(const NAME &other) const { \
        static_cast<const Enum<__VA_ARGS__> *>(this)->__detect_trait_eq(other); \
    }

#define CRUST_ENUM_VARIANTS(NAME, ...) \
    struct NAME : \
    public Tuple<__VA_ARGS__> { \
        template<class ...Args> \
        explicit constexpr NAME(Args ...args) : \
                Tuple<__VA_ARGS__>{::crust::forward<Args>(args)...} {} \
        void __detect_trait_partial_eq(const NAME &other) const { \
            static_cast<const Tuple<__VA_ARGS__> *>(this)->__detect_trait_partial_eq(other); \
        } \
        void __detect_trait_eq(const NAME &other) const { \
            static_cast<const Tuple<__VA_ARGS__> *>(this)->__detect_trait_eq(other); \
        } \
    }
}


#endif //CRUST_UTILITY_HPP
