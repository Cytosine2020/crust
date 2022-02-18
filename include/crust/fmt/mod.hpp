#ifndef CRUST_FMT_MOD_HPP
#define CRUST_FMT_MOD_HPP


#include "crust/utility.hpp"


namespace crust {
namespace fmt {
class Formatter {};

CRUST_TRAIT(Debug) {
  CRUST_TRAIT_USE_SELF(Debug);

  bool fmt_debug(Formatter & fmt);
};

CRUST_TRAIT(Display) {
  CRUST_TRAIT_USE_SELF(Display);

  bool fmt_display(Formatter & fmt);
};
} // namespace fmt
} // namespace crust


#endif // CRUST_FMT_MOD_HPP
