#ifndef CRUST_OPS_RANGE_HPP
#define CRUST_OPS_RANGE_HPP


#include "crust/enum.hpp"
#include "crust/utility.hpp"


namespace crust {
namespace range {
struct RangeFull {};

template <class T>
struct Range {
  T start;
  T end;

  constexpr Range(T start, T end) : start{start}, end{end} {}

  constexpr bool is_empty() const { return start >= end; }
};

template <class T>
struct RangeFrom {
  T start;

  explicit constexpr RangeFrom(T start) : start{start} {}
};

template <class T>
struct RangeTo {
  T end;

  explicit constexpr RangeTo(T end) : end{end} {}
};
} // namespace range
} // namespace crust


#endif // CRUST_OPS_RANGE_HPP
