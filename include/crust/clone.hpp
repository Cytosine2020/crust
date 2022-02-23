#ifndef CRUST_CLONE_HPP
#define CRUST_CLONE_HPP


#include "crust/utility.hpp"


namespace crust {

namespace clone {
template <class T>
T clone(const T &self) {
  return self.clone();
}

CRUST_TRAIT(Clone) {
  CRUST_TRAIT_USE_SELF(Clone);

  Self clone() const { return self(); }

  void clone_from(const Self &other) { self() = clone::clone(other); }
};
} // namespace clone
} // namespace crust


#endif // CRUST_CLONE_HPP
