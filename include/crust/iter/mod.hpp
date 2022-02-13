#ifndef CRUST_ITER_MOD_HPP
#define CRUST_ITER_MOD_HPP


#include "crust/option.hpp"
#include "crust/tuple.hpp"
#include "crust/utility.hpp"


namespace crust {
namespace iter {
CRUST_TRAIT(Iterator, class Item) {
  CRUST_TRAIT_REQUIRE(Iterator);

  Option<Item> next();

  constexpr Tuple<usize, Option<usize>> size_hint() const {
    return make_tuple<usize, Option<usize>>(0, None{});
  }

  template <class B, class F>
  B fold(B && init, ops::Fn<F, B(B &&, Item &&)> f) {
    B accum = forward(init);

    Item x;
    while ((let<Some<Item>>(x) = self().next())) {
      accum = f(move(accum), move(x));
    }

    return accum;
  }
};
} // namespace iter
} // namespace crust


#endif // CRUST_ITER_MOD_HPP
