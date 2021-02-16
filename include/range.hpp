#ifndef CRUST_RANGE_HPP
#define CRUST_RANGE_HPP


#include "utility.hpp"


namespace crust {
class RangeFull {
};

template<class T>
class Range {
public:
    T start;
    T end;

    Range(T start, T end) : start{start}, end{end} {}

    bool is_empty() const { return start >= end; }
};

template<class T>
class RangeFrom {
public:
    T start;

    explicit RangeFrom(T start) : start{start} {}
};

template<class T>
class RangeTo {
public:
    T end;

    explicit RangeTo(T end) : end{end} {}
};
}


#endif //CRUST_RANGE_HPP
