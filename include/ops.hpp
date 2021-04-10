#ifndef CRUST_OPS_HPP
#define CRUST_OPS_HPP


#include "utility.hpp"


namespace crust {
namespace index {
CRUST_TRAIT(Index, class Idx, class T)
public:
    const T &index(Idx idx) const;

    T &index_mut(Idx idx);

    const T &operator[](Idx idx) const { return self().index(idx); }

    T &operator[](Idx idx) { return self().index_mut(idx); }
};
}
}


#endif //CRUST_OPS_HPP
