#ifndef CRUST_OPS_MOD_HPP
#define CRUST_OPS_MOD_HPP


#include "crust/utility.hpp"


namespace crust {
namespace index {
CRUST_TRAIT(Index, class Idx, class T) {
  CRUST_TRAIT_REQUIRE(Index);

  const T &index(Idx idx) const;

  T &index_mut(Idx idx);

  const T &operator[](Idx idx) const { return self().index(idx); }

  T &operator[](Idx idx) { return self().index_mut(idx); }
};
} // namespace index
} // namespace crust


#endif // CRUST_OPS_MOD_HPP
