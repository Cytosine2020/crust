#include "crust.hpp"


using namespace crust;


class A {
public:
    A() noexcept { printf("%p %s\n", this, __PRETTY_FUNCTION__); }

    A(const A &) { printf("%p %s\n", this, __PRETTY_FUNCTION__); }

    A(A &&) noexcept { printf("%p %s\n", this, __PRETTY_FUNCTION__); }

    A &operator=(const A &) {
        printf("%p %s\n", this, __PRETTY_FUNCTION__);
        return *this;
    }

    A &operator=(A &&) noexcept {
        printf("%p %s\n", this, __PRETTY_FUNCTION__);
        return *this;
    }

    void operator()() { printf("%p %s\n", this, __PRETTY_FUNCTION__); }

    void operator()() const { printf("%p %s\n", this, __PRETTY_FUNCTION__); }

    ~A() { printf("%p %s\n", this, __PRETTY_FUNCTION__); }
};

class B {
public:
    B() noexcept { printf("%p %s\n", this, __PRETTY_FUNCTION__); }

    B(const B &) { printf("%p %s\n", this, __PRETTY_FUNCTION__); }

    B(B &&) noexcept { printf("%p %s\n", this, __PRETTY_FUNCTION__); }

    B &operator=(const B &) {
        printf("%p %s\n", this, __PRETTY_FUNCTION__);
        return *this;
    }

    B &operator=(B &&) noexcept {
        printf("%p %s\n", this, __PRETTY_FUNCTION__);
        return *this;
    }

    void operator()() { printf("%p %s\n", this, __PRETTY_FUNCTION__); }

    void operator()() const { printf("%p %s\n", this, __PRETTY_FUNCTION__); }

    ~B() { printf("%p %s\n", this, __PRETTY_FUNCTION__); }
};

void fn_c() { printf("%s\n", __PRETTY_FUNCTION__); }

int main() {
    {
        constexpr usize BUFFER_SIZE = 10;

        u8 buffer[BUFFER_SIZE]{};

        auto slice = Slice<u8>::from_raw_parts(buffer, BUFFER_SIZE);

        printf("slice[1]: %d\n", slice[1]);
    }

    {
        CRUST_ASSERT(make_tuple(1, 'a') == max_by_key(make_tuple(0, 'b'), make_tuple(1, 'a'),
                                                      make_fn([](const Tuple<i32, char> &value) {
                                                          return value.get<0>();
                                                      })));

        CRUST_ASSERT(make_tuple(0, 'b') == max_by_key(make_tuple(0, 'b'), make_tuple(1, 'a'),
                                                      make_fn([](const Tuple<i32, char> &value) {
                                                          return value.get<1>();
                                                      })));
    }

    {
        CRUST_STATIC_ASSERT(!CRUST_DERIVE((Tuple<A, B>), PartialEq));
        CRUST_STATIC_ASSERT(!CRUST_DERIVE((Tuple<A, B>), Eq));

        CRUST_STATIC_ASSERT(std::is_literal_type<Tuple<>>::value);

        auto tuple = make_tuple(0, 'a');

        CRUST_ASSERT(tuple.get<0>() == 0);
        CRUST_ASSERT(tuple.get<1>() == 'a');

        ++tuple.get<0>();
        ++tuple.get<1>();

        CRUST_ASSERT(std::get<0>(tuple) == 1);
        CRUST_ASSERT(std::get<1>(tuple) == 'b');

        CRUST_STATIC_ASSERT(make_tuple() == make_tuple());

        CRUST_ASSERT(make_tuple(true) != make_tuple(false));
        CRUST_ASSERT(make_tuple(1) > make_tuple(0));
        CRUST_ASSERT(make_tuple(0, 'b') > make_tuple(0, 'a'));

        CRUST_ASSERT(make_tuple(0, 1).cmp(make_tuple(0, 0)) == Ordering::greater());
    }

    printf("=====\n");

    {
        auto a = make_fn_mut<void()>(B{});
        a();
    }

    printf("=====\n");

    {
        auto c = CRUST_MAKE_FN_MUT(fn_c);
        c();
    }

    printf("=====\n");

    {
        auto lambda = make_fn_mut([]() mutable { printf("%s\n", __PRETTY_FUNCTION__); });
        lambda();
    }

    printf("=====\n");

    {
        DynFnMut<void()> dyn_b = make_dyn_fn_mut<void()>(B{});
        dyn_b();
    }

    printf("=====\n");

    {
        DynFnMut<void()> dyn_c = CRUST_MAKE_DYN_FN_MUT(fn_c);
        dyn_c();
    }

    printf("=====\n");

    {
        DynFnMut<void()> lambda = make_dyn_fn_mut([]() mutable {
            printf("%s\n", __PRETTY_FUNCTION__);
        });
        lambda();
    }

    printf("=====\n");

    {
        auto b = make_fn<void()>(B{});
        b();
    }

    printf("=====\n");

    {
        auto c = CRUST_MAKE_FN(fn_c);
        c();
    }

    printf("=====\n");

    {
        auto lambda = make_fn([]() { printf("%s\n", __PRETTY_FUNCTION__); });
        lambda();
    }

    printf("=====\n");

    {
        DynFn<void()> dyn_b = make_dyn_fn<void()>(B{});
        dyn_b();
    }

    printf("=====\n");

    {
        DynFn<void()> dyn_c = CRUST_MAKE_DYN_FN(fn_c);
        dyn_c();
    }

    printf("=====\n");

    {
        DynFn<void()> lambda = make_dyn_fn([]() { printf("%s\n", __PRETTY_FUNCTION__); });
        lambda();
    }

    printf("=====\n");

    {
        auto a = Ordering::equal();

        CRUST_ASSERT(a.then_with(make_fn([]() { return Ordering::less(); })) == Ordering::less());
    }

    {
        struct VisitA {
            void operator()(A &) {}

            void operator()(B &) { crust_panic(""); }
        };

        struct VisitB {
            void operator()(A &) { crust_panic(""); }

            void operator()(B &) {}
        };

        Enum<A, B> a{};

        CRUST_STATIC_ASSERT(!CRUST_DERIVE((Enum<A, B>), PartialEq));
        CRUST_STATIC_ASSERT(!CRUST_DERIVE((Enum<A, B>), Eq));
        CRUST_STATIC_ASSERT(!std::is_trivially_copyable<Enum<A, B>>::value);

        a = Enum<A, B>{A{}};

        a.visit<VisitA>();

        a = Enum<A, B>{B{}};

        a.visit<VisitB>();

        a = A{};

        a.visit<VisitA>();

        a = B{};

        a.visit<VisitB>();
    }

    printf("=====\n");

    {
        struct A {
        };

        struct B {
        };

        struct C {
        };

        struct D {
        };

        struct E {
        };

        struct F {
        };

        struct Visit {
            void operator()(const A &) { printf("visit A\n"); }

            void operator()(const B &) { printf("visit B\n"); }

            void operator()(const C &) { printf("visit C\n"); }

            void operator()(const D &) { printf("visit D\n"); }

            void operator()(const E &) { printf("visit E\n"); }

            void operator()(const F &) { printf("visit F\n"); }
        };

        Enum<A, B, C, D, E, F> a{};

        CRUST_STATIC_ASSERT(std::is_trivially_copyable<Enum<A, B, C, D, E, F>>::value);
        CRUST_STATIC_ASSERT(std::is_literal_type<Enum<A, B, C, D, E, F>>::value);

        a = A{};
        a.visit<Visit>();
        a = B{};
        a.visit<Visit>();
        a = C{};
        a.visit<Visit>();
        a = D{};
        a.visit<Visit>();
        a = E{};
        a.visit<Visit>();
        a = F{};
        a.visit<Visit>();
    }

    {
        using Enumerate = Enum<i32, char>;

        CRUST_ASSERT(Enumerate{0} == Enumerate{0});

        CRUST_ASSERT(Enumerate{0} != Enumerate{'a'});

        CRUST_ASSERT(Enumerate{0} != Enumerate{1});
    }
}
