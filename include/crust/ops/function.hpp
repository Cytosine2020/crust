#ifndef CRUST_OPS_FUNCTION_HPP
#define CRUST_OPS_FUNCTION_HPP


#include "crust/tuple_decl.hpp"
#include "crust/utility.hpp"


namespace crust {
namespace ops {
namespace _impl_fn {
template <
    class Self,
    class F = decltype(&Self::operator()),
    F f = &Self::operator()>
struct MemFnClosure;

template <class Self, class Ret, class... Args, Ret (Self::*f)(Args...) const>
struct MemFnClosure<Self, Ret (Self::*)(Args...) const, f> :
    TmplType<Ret(Args...)> {};

template <class Self, class Ret, class... Args, Ret (Self::*f)(Args...)>
struct MemFnClosure<Self, Ret (Self::*)(Args...), f> :
    TmplType<Ret(Args...)> {};

template <class F, F f>
struct RawMemFn;

template <class Self, class Ret, class... Args, Ret (Self::*f)(Args...) const>
struct RawMemFn<Ret (Self::*)(Args...) const, f> {
  constexpr Ret operator()(const Self &self, Args... args) const {
    return (self.*f)(forward<Args>(args)...);
  }
};

template <class Self, class Ret, class... Args, Ret (Self::*f)(Args...)>
struct RawMemFn<Ret (Self::*)(Args...), f> {
  constexpr Ret operator()(Self &self, Args... args) const {
    return (self.*f)(forward<Args>(args)...);
  }
};

template <class F, F *f>
struct RawFn;

template <class Ret, class... Args, Ret (*f)(Args...)>
struct RawFn<Ret(Args...), f> {
  constexpr Ret operator()(Args... args) const {
    return f(forward<Args>(args)...);
  }
};
} // namespace _impl_fn

template <class Self, class F = typename _impl_fn::MemFnClosure<Self>::Result>
struct Fn;

template <class Self, class Ret, class... Args>
struct Fn<Self, Ret(Args...)> {
private:
  crust_static_assert(!IsConstOrRefVal<Self>::result);

  Self self;

public:
  constexpr Fn(Self &&self) : self{move(self)} {}

  constexpr Ret operator()(Args... args) const {
    return self(forward<Args>(args)...);
  }
};

template <class F, F *f>
crust_always_inline Fn<_impl_fn::RawFn<F, f>> bind(TmplVal<F *, f>) {
  return Fn<_impl_fn::RawFn<F, f>>{_impl_fn::RawFn<F, f>{}};
}

template <class T, class F, F *f>
crust_always_inline Fn<_impl_fn::RawFn<F, f>, T> bind(TmplVal<F *, f>) {
  return Fn<_impl_fn::RawFn<F, f>, T>{_impl_fn::RawFn<F, f>{}};
}

template <class F, F f>
crust_always_inline Fn<_impl_fn::RawMemFn<F, f>> bind(TmplVal<F, f>) {
  return Fn<_impl_fn::RawMemFn<F, f>>{_impl_fn::RawMemFn<F, f>{}};
}

template <class T, class F, F f>
crust_always_inline Fn<_impl_fn::RawMemFn<F, f>, T> bind(TmplVal<F, f>) {
  return Fn<_impl_fn::RawMemFn<F, f>, T>{_impl_fn::RawMemFn<F, f>{}};
}

template <class T>
crust_always_inline constexpr Fn<T> bind(T &&f) {
  return Fn<T>{forward<T>(f)};
}

template <class F, class T>
crust_always_inline constexpr Fn<T, F> bind(T &&f) {
  return Fn<T, F>{forward<T>(f)};
}

template <class Self, class F = typename _impl_fn::MemFnClosure<Self>::Result>
struct FnMut;

template <class Self, class Ret, class... Args>
struct FnMut<Self, Ret(Args...)> {
private:
  crust_static_assert(!IsConstOrRefVal<Self>::result);

  Self self;

public:
  constexpr FnMut(Self &&self) : self{move(self)} {}

  crust_cxx14_constexpr Ret operator()(Args... args) {
    return self(forward<Args>(args)...);
  }
};

template <class F, F *f>
crust_always_inline FnMut<_impl_fn::RawFn<F, f>> bind_mut(TmplVal<F *, f>) {
  return FnMut<_impl_fn::RawFn<F, f>>{_impl_fn::RawFn<F, f>{}};
}

template <class T, class F, F *f>
crust_always_inline FnMut<_impl_fn::RawFn<F, f>, T> bind_mut(TmplVal<F *, f>) {
  return FnMut<_impl_fn::RawFn<F, f>, T>{_impl_fn::RawFn<F, f>{}};
}

template <class F, F f>
crust_always_inline FnMut<_impl_fn::RawMemFn<F, f>> bind_mut(TmplVal<F, f>) {
  return FnMut<_impl_fn::RawMemFn<F, f>>{_impl_fn::RawMemFn<F, f>{}};
}

template <class T, class F, F f>
crust_always_inline FnMut<_impl_fn::RawMemFn<F, f>, T> bind_mut(TmplVal<F, f>) {
  return FnMut<_impl_fn::RawMemFn<F, f>, T>{_impl_fn::RawMemFn<F, f>{}};
}

template <class T>
crust_always_inline constexpr FnMut<T> bind_mut(T &&f) {
  return FnMut<T>{forward<T>(f)};
}

template <class F, class T>
crust_always_inline constexpr FnMut<T, F> bind_mut(T &&f) {
  return FnMut<T, F>{forward<T>(f)};
}

namespace _impl_fn {
template <class Base, class Self>
struct DestructorStaticWrapper {
  static crust_cxx14_constexpr void inner(Base *self) {
    reinterpret_cast<Self *>(self)->~Self();
  }
};

template <class Ret, class... Args>
struct FnVTable {
  void (*drop)(void *);
  usize size;
  usize align;
  Ret (*call)(const void *, Args...);
};

template <class Self, class Ret, class... Args>
struct StaticFnVTable {
  static const FnVTable<Ret, Args...> vtable;

  static constexpr Ret call(const void *self, Args &&...args) {
    return (*reinterpret_cast<const Self *>(self))(forward<Args>(args)...);
  }
};

template <class Self, class Ret, class... Args>
const FnVTable<Ret, Args...> StaticFnVTable<Self, Ret, Args...>::vtable{
    DestructorStaticWrapper<void, Self>::inner,
    sizeof(Self),
    alignof(Self),
    call,
};

template <class Ret, class... Args>
struct FnMutVTable {
  void (*drop)(void *);
  usize size;
  usize align;
  Ret (*call)(void *, Args...);
};

template <class Self, class Ret, class... Args>
struct StaticFnMutVTable {
  static const FnMutVTable<Ret, Args...> vtable;

  static crust_cxx14_constexpr Ret call(void *self, Args... args) {
    return (*reinterpret_cast<Self *>(self))(forward<Args>(args)...);
  }
};

template <class Self, class Ret, class... Args>
const FnMutVTable<Ret, Args...> StaticFnMutVTable<Self, Ret, Args...>::vtable{
    DestructorStaticWrapper<void, Self>::inner,
    sizeof(Self),
    alignof(Self),
    call,
};
} // namespace _impl_fn

template <class F>
struct DynFn;

template <class F>
struct DynFnMut;

template <class Ret, class... Args>
struct DynFn<Ret(Args...)> { // TODO: optimize for zero sized type
private:
  void *self;
  const _impl_fn::FnVTable<Ret, Args...> *vtable;

  void drop() {
    if (self != nullptr) {
      vtable->drop(self);
      delete reinterpret_cast<char *>(self);
      self = nullptr;
    }
  }

  void move_from(DynFn &&other) {
    self = other.self;
    vtable = other.vtable;
    other.self = nullptr;
  }

public:
  template <class Self>
  constexpr DynFn(Self &&self) :
      self{new Self{forward<Self>(self)}},
      vtable{&_impl_fn::StaticFnVTable<Self, Ret, Args...>::vtable} {}

  template <class F, F *f>
  constexpr DynFn(TmplVal<F *, f>) : DynFn{_impl_fn::RawFn<F, f>{}} {}

  DynFn(DynFn &&other) noexcept { move_from(move(other)); }

  DynFn &operator=(DynFn &&other) noexcept {
    if (this != &other) {
      drop();
      move_from(move(other));
    }

    return *this;
  }

  constexpr Ret operator()(Args &&...args) const {
    return vtable->call(self, forward<Args>(args)...);
  }

  ~DynFn() { drop(); }
};

template <class Ret, class... Args>
struct DynFnMut<Ret(Args...)> {
private:
  void *self;
  const _impl_fn::FnMutVTable<Ret, Args...> *vtable;

  void drop() {
    if (self != nullptr) {
      vtable->drop(self);
      delete reinterpret_cast<char *>(self);
      self = nullptr;
    }
  }

  void move_from(DynFnMut &&other) {
    self = other.self;
    vtable = other.vtable;
    other.self = nullptr;
  }

public:
  template <class Self>
  constexpr DynFnMut(Self &&self) :
      self{new Self{forward<Self>(self)}},
      vtable{&_impl_fn::StaticFnMutVTable<Self, Ret, Args...>::vtable} {}

  template <class F, F *f>
  constexpr DynFnMut(TmplVal<F *, f>) : DynFnMut{_impl_fn::RawFn<F, f>{}} {}

  DynFnMut(DynFnMut &&other) noexcept { move_from(move(other)); }

  DynFnMut &operator=(DynFnMut &&other) noexcept {
    if (this != &other) {
      drop();
      move_from(move(other));
    }

    return *this;
  }

  crust_cxx14_constexpr Ret operator()(Args... args) {
    return vtable->call(self, forward<Args>(args)...);
  }

  ~DynFnMut() { drop(); }
};
} // namespace ops
} // namespace crust


#endif // CRUST_OPS_FUNCTION_HPP
