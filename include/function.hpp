#ifndef CRUST_FUNCTION_HPP
#define CRUST_FUNCTION_HPP


#include "utility.hpp"


namespace crust {
template<class Base, class F, F f>
struct MemberFnStaticWrapper;

template<class Base, class Self, class Ret, class ...Args, Ret (Self::*f)(Args...) const>
struct MemberFnStaticWrapper<Base, Ret (Self::*)(Args...) const, f> {
    using Inner = Ret(const Base *, Args...);

    constexpr static Ret inner(const Base *self, Args...args) {
        return (reinterpret_cast<const Self *>(self)->*f)(forward<Args>(args)...);
    }
};

template<class Base, class Self, class Ret, class ...Args, Ret (Self::*f)(Args...)>
struct MemberFnStaticWrapper<Base, Ret (Self::*)(Args...), f> {
    using Inner = Ret(Base *, Args...);

    static constexpr Ret inner(Base *self, Args...args) {
        return (reinterpret_cast<Self *>(self)->*f)(forward<Args>(args)...);
    }
};

#define CRUST_MEMBER_FN_STATIC_WRAPPER(BASE, PTR) \
    ::crust::MemberFnStaticWrapper<BASE, decltype(PTR), PTR>

template<class Base, class Self>
struct DestructorStaticWrapper {
    using Inner = void(Base *);

    static CRUST_CXX14_CONSTEXPR void inner(Base *self) {
        reinterpret_cast<Self *>(self)->~Self();
    }
};

template<class F, F f>
struct MemberFnClosureWrapper;

template<class Self, class Return, class ...Args, Return (Self::*f)(Args...) const>
struct MemberFnClosureWrapper<Return(Self::*)(Args...) const, f> {
    using Inner = Return(Args...);
};

template<class Self, class Return, class ...Args, Return (Self::*f)(Args...)>
struct MemberFnClosureWrapper<Return(Self::*)(Args...), f> {
    using Inner = Return(Args...);
};

#define CRUST_MEMBER_FN_CLOSURE_WRAPPER(PTR) \
    ::crust::MemberFnClosureWrapper<decltype(PTR), PTR>


namespace __impl_raw_fn {
template<class F, F *f>
class RawFn;

template<class Result, class ...Args, Result(*f)(Args...)>
class RawFn<Result(Args...), f> {
public:
    CRUST_CXX14_CONSTEXPR Result operator()(Args ...args) { return f(args...); }

    constexpr Result operator()(Args ...args) const { return f(args...); }
};


template<class Result, class ...Args>
struct FnMutVTable {
    void (*drop)(void *);

    usize size, align;

    Result (*call_mut)(void *, Args...);
};

template<class Self, class Result, class ...Args>
struct StaticFnMutVTable {
    const static FnMutVTable<Result, Args...> vtable;

    static CRUST_CXX14_CONSTEXPR Result call_mut(void *self, Args ...args) {
        (*reinterpret_cast<Self *>(self))(forward<Args>(args)...);
    }
};

template<class Self, class Result, class ...Args>
const FnMutVTable<Result, Args...> StaticFnMutVTable<Self, Result, Args...>::vtable{
        DestructorStaticWrapper<void, Self>::inner,
        sizeof(Self),
        alignof(Self),
        call_mut,
};

template<class Result, class ...Args>
struct FnVTable {
    void (*drop)(void *);

    usize size, align;

    Result (*call)(const void *, Args...);
};

template<class Self, class Result, class ...Args>
struct StaticFnVTable {
    const static FnVTable<Result, Args...> vtable;

    static CRUST_CXX14_CONSTEXPR Result call(const void *self, Args ...args) {
        (*reinterpret_cast<const Self *>(self))(forward<Args>(args)...);
    }
};

template<class Self, class Result, class ...Args>
const FnVTable<Result, Args...> StaticFnVTable<Self, Result, Args...>::vtable{
        DestructorStaticWrapper<void, Self>::inner,
        sizeof(Self), alignof(Self),
        call,
};
}

template<class Self, class F = typename CRUST_MEMBER_FN_CLOSURE_WRAPPER(&Self::operator())::Inner>
class FnMut;

template<class Self, class Result, class ...Args>
class FnMut<Self, Result(Args...)> {
private:
    Self self;

public:
    constexpr FnMut(const Self &self) : self{self} {}

    constexpr FnMut(Self &&self) : self{move(self)} {}

    CRUST_CXX14_CONSTEXPR Result operator()(Args ...args) {
        return self(forward<Args>(args)...);
    }
};

template<class T>
constexpr FnMut<T> make_fn_mut(T &&f) { return FnMut<T>(forward<T>(f)); }

template<class F, class T>
constexpr FnMut<T, F> make_fn_mut(T &&f) { return FnMut<T, F>(forward<T>(f)); }

template<class F, F *f>
constexpr FnMut<__impl_raw_fn::RawFn<F, f>, F> make_fn_mut() {
    return FnMut<__impl_raw_fn::RawFn<F, f>, F>(__impl_raw_fn::RawFn<F, f>{});
}

#define CRUST_MAKE_FN_MUT(f) ::crust::make_fn_mut<decltype(f), f>()

template<class Self, class F = typename CRUST_MEMBER_FN_CLOSURE_WRAPPER(&Self::operator())::Inner>
class Fn;

template<class Self, class Result, class ...Args>
class Fn<Self, Result(Args...)> {
private:
    Self self;

public:
    constexpr Fn(const Self &self) : self{self} {}

    constexpr Fn(Self &&self) : self{move(self)} {}

    CRUST_CXX14_CONSTEXPR Result operator()(Args ...args) const {
        return self(forward<Args>(args)...);
    }
};

template<class T, class Function>
class FnMut<FnMut<T, Function>, Function> {
};

template<class T, class Function>
class Fn<Fn<T, Function>, Function> {
};

template<class T, class Function>
class FnMut<Fn<T, Function>, Function> {
};

template<class T, class Function>
class Fn<FnMut<T, Function>, Function> {
};

template<class T>
constexpr Fn<T> make_fn(T &&f) { return Fn<T>(forward<T>(f)); }

template<class F, class T>
constexpr Fn<T, F> make_fn(T &&f) { return Fn<T, F>(forward<T>(f)); }

template<class F, F *f>
constexpr Fn<__impl_raw_fn::RawFn<F, f>, F> make_fn() {
    return Fn<__impl_raw_fn::RawFn<F, f>, F>(__impl_raw_fn::RawFn<F, f>{});
}

#define CRUST_MAKE_FN(f) ::crust::make_fn<decltype(f), f>()

template<class F>
class DynFnMut;

template<class Result, class ...Args>
class DynFnMut<Result(Args...)> {
private:
    using VTable = __impl_raw_fn::FnMutVTable<Result, Args...>;

    template<class Self>
    using StaticVTable = __impl_raw_fn::StaticFnMutVTable<Self, Result, Args...>;

    void *self;
    const VTable *vtable;

    void drop() {
        vtable->drop(self);
        delete reinterpret_cast<char *>(self);
        self = nullptr;
    }

public:
    template<class Self>
    explicit constexpr DynFnMut(Self &&self) noexcept:
            self{new Self{forward<Self>(self)}}, vtable{&StaticVTable<Self>::vtable} {}

    template<class F, F f>
    explicit constexpr DynFnMut(__impl_raw_fn::RawFn<F, f>) noexcept:
            self{nullptr}, vtable{&StaticVTable<__impl_raw_fn::RawFn<F, f>>::vtable} {}

    DynFnMut(DynFnMut &&other) noexcept {
        if (this != &other) {
            self = other.self;
            vtable = other.vtable;
            other.self = nullptr;
        }
    }

    DynFnMut &operator=(DynFnMut &&other) noexcept {
        if (this != &other) {
            if (self != nullptr) { drop(); }
            self = other.self;
            vtable = other.vtable;
            other.self = nullptr;
        }
    }

    CRUST_CXX14_CONSTEXPR Result operator()(Args ...args) {
        return vtable->call_mut(self, forward<Args>(args)...);
    }

    ~DynFnMut() { if (self != nullptr) { drop(); }}
};

template<class T>
constexpr DynFnMut<typename CRUST_MEMBER_FN_CLOSURE_WRAPPER(&T::operator())::Inner>
make_dyn_fn_mut(T &&f) {
    using Function = typename CRUST_MEMBER_FN_CLOSURE_WRAPPER(&T::operator())::Inner;
    return DynFnMut<Function>(forward<T>(f));
}

template<class F, class T>
constexpr DynFnMut<F> make_dyn_fn_mut(T &&f) { return DynFnMut<F>(forward<T>(f)); }

template<class F, F *f>
constexpr DynFnMut<F> make_dyn_fn_mut() { return DynFnMut<F>(__impl_raw_fn::RawFn<F, f>{}); }

#define CRUST_MAKE_DYN_FN_MUT(f) ::crust::make_dyn_fn_mut<decltype(f), f>()


template<class F>
class DynFn;

template<class Result, class ...Args>
class DynFn<Result(Args...)> {
private:
    using VTable = __impl_raw_fn::FnVTable<Result, Args...>;

    template<class Self>
    using StaticVTable = __impl_raw_fn::StaticFnVTable<Self, Result, Args...>;

    void *self;
    const VTable *vtable;

    void drop() {
        vtable->drop(self);
        delete reinterpret_cast<char *>(self);
        self = nullptr;
    }

public:
    template<class Self>
    explicit constexpr DynFn(Self &&self) noexcept:
            self{new Self{forward<Self>(self)}}, vtable{&StaticVTable<Self>::vtable} {}

    template<class F, F f>
    explicit constexpr DynFn(__impl_raw_fn::RawFn<F, f>) noexcept:
            self{nullptr}, vtable{&StaticVTable<__impl_raw_fn::RawFn<F, f>>::vtable} {}

    DynFn(DynFn &&other) noexcept {
        if (this != &other) {
            self = other.self;
            vtable = other.vtable;
            other.self = nullptr;
        }
    }

    DynFn &operator=(DynFn &&other) noexcept {
        if (this != &other) {
            if (self != nullptr) { drop(); }
            self = other.self;
            vtable = other.vtable;
            other.self = nullptr;
        }
    }

    constexpr Result operator()(Args ...args) const {
        return vtable->call(self, forward<Args>(args)...);
    }

    ~DynFn() { if (self != nullptr) { drop(); }}
};

template<class T>
constexpr DynFn<typename CRUST_MEMBER_FN_CLOSURE_WRAPPER(&T::operator())::Inner>
make_dyn_fn(T &&f) {
    return DynFn<typename CRUST_MEMBER_FN_CLOSURE_WRAPPER(&T::operator())::Inner>(forward<T>(f));
}

template<class F, class T>
constexpr DynFn<F> make_dyn_fn(T &&f) { return DynFn<F>(forward<T>(f)); }

template<class F, F *f>
constexpr DynFn<F> make_dyn_fn() { return DynFn<F>(__impl_raw_fn::RawFn<F, f>{}); }

#define CRUST_MAKE_DYN_FN(f) ::crust::make_dyn_fn<decltype(f), f>()
}


#endif //CRUST_FUNCTION_HPP
