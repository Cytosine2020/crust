#ifndef CRUST_OPTION_DECLARE_HPP
#define CRUST_OPTION_DECLARE_HPP


#include "utility.hpp"
#include "function.hpp"
#include "enum.hpp"
#include "cmp_declare.hpp"
#include "tuple_declare.hpp"


namespace crust {
template<class T>
struct Some;

struct None;

template<class T>
class Option : public Enum<Some<T>, None> {
public:
    CRUST_ENUM_USE_BASE(Option, (Enum<Some<T>, None>));

private:
    struct IsSome {
        bool operator()(const Some<T> &) { return true; }

        bool operator()(const None &) { return false; }
    };

public:
    bool is_some() const { return this->template visit<IsSome, bool>(); }

    bool is_none() const { return !is_some(); }

private:
    struct Contains {
        const T &other;

        bool operator()(const Some<T> &value) { return value.template get<0>() == other; }

        bool operator()(const None &) { return false; }
    };

public:
    bool contains(const T &other) const { return this->template visit<Contains, bool>({other}); }

private:
    struct AsPtr;

public:
    Option<const T *> as_ptr() const { return this->template visit<AsPtr, Option<const T *>>(); }

private:
    struct AsMutPtr;

public:
    Option<T *> as_mut_ptr() { return this->template visit<AsMutPtr, Option<T *>>(); }

private:
    struct Unwrap {
        T &&operator()(Some<T> &&value) { return move(value); }

        T &&operator()(None &&) { crust_panic("called `Option::unwrap()` on a `None` value"); }
    };

public:
    T &&unwrap() { return this->template visit_move<Unwrap, T &&>(); }

private:
    struct UnwrapOr {
        T &&d;

        T &&operator()(Some<T> &&value) { return move(value); }

        T &&operator()(None &&) { return move(d); }
    };

public:
    T &&unwrap_or(T &&d) { return this->template visit_move<UnwrapOr, T &&>({d}); }

private:
    template<class F, class Arg, class U>
    struct Map;

public:
    template<class F, class Arg, class U>
    Option<U> map(const Fn<F, U(Arg)> &f) const {
        return this->template visit<Map<F, Arg, U>, Option<U>>({f});
    }

private:
    template<class F, class Arg, class U>
    struct MapOr {
        U &&d;
        const Fn<F, U(Arg)> &f;

        U operator()(const Some<T> &value) { return f(value.template get<0>()); }

        U operator()(const None &) { return forward<U>(d); }
    };

public:
    template<class F, class Arg, class U>
    U map_or(U &&d, const Fn<F, U(Arg)> &f) const {
        return this->template visit<MapOr<F, Arg, U>, U>({d, f});
    }
};
}


#endif //CRUST_OPTION_DECLARE_HPP
