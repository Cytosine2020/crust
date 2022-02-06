#ifndef _CRUST_INCLUDE_ITER_HPP
#define _CRUST_INCLUDE_ITER_HPP


#include "utility.hpp"

#include "option.hpp"
#include "tuple.hpp"


namespace crust {
namespace iter {
CRUST_TRAIT(Iterator, class Item) {
  CRUST_TRAIT_REQUIRE(Iterator);

  Option<Item> next();

  constexpr Tuple<usize, Option<usize>> size_hint() const {
    return make_tuple<usize, Option<usize>>(0, None{});
  }

  template<class B, class F>
  B fold(B &&init, ops::Fn<F, B(B &&, Item &&)> f) {
    B accum = forward(init);

    Item x;
    while ((let<Some<Item>>(x) = self().next())) {
      accum = f(move(accum), move(x));
    }

    return accum;
  }
};
}
}


#endif //_CRUST_INCLUDE_ITER_HPP
