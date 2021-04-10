#ifndef CRUST_ITER_HPP
#define CRUST_ITER_HPP


#include "utility.hpp"

#include "option.hpp"
#include "tuple.hpp"


namespace crust {
namespace iter {
CRUST_TRAIT(Iterator, class Item)
public:
    Option<Item> next();

    constexpr Tuple<usize, Option<usize>> size_hint() const {
        return make_tuple<usize, Option<usize>>(0, None{});
    }

    template<class B, class F>
    B fold(B &&init, Fn<F, B &&(B &&, Item &&)> &&f) {
        B accum = std::forward(init);

        Item x;
        while ((let<Some<Item>>(x) = self().next())) {
            accum = f(std::move(accum), std::move(x));
        }

        return accum;
    }
};
}
}


#endif //CRUST_ITER_HPP
