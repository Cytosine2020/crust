#ifndef _CRUST_INCLUDE_FMT_HPP
#define _CRUST_INCLUDE_FMT_HPP


#include "utility.hpp"


namespace crust {
namespace fmt {
class Formatter {};

CRUST_TRAIT(Debug) {
  CRUST_TRAIT_REQUIRE(Debug);

  bool fmt_debug(Formatter & fmt);
};

CRUST_TRAIT(Display) {
  CRUST_TRAIT_REQUIRE(Display);

  bool fmt_display(Formatter & fmt);
};
} // namespace fmt
} // namespace crust


#endif //_CRUST_INCLUDE_FMT_HPP
