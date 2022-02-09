#ifndef CRUST_OPS_RANGE_HPP
#define CRUST_OPS_RANGE_HPP


#include "crust/enum.hpp"
#include "crust/utility.hpp"


namespace crust {
namespace range {
class RangeFull {};

template <class T>
class Range {
public:
  T start;
  T end;

  constexpr Range(T start, T end) : start{start}, end{end} {}

  constexpr bool is_empty() const { return start >= end; }
};

template <class T>
class RangeFrom {
public:
  T start;

  explicit constexpr RangeFrom(T start) : start{start} {}
};

template <class T>
class RangeTo {
public:
  T end;

  explicit constexpr RangeTo(T end) : end{end} {}
};
} // namespace range
} // namespace crust


#endif // CRUST_OPS_RANGE_HPP
