#ifndef CRUST_FMT_HPP
#define CRUST_FMT_HPP


#include "utility.hpp"


namespace crust {
class Formatter {

};

CRUST_TRAIT(Debug)
public:
    bool fmt_debug(Formatter &fmt);
};

CRUST_TRAIT(Display)
public:
    bool fmt_display(Formatter &fmt);
};
}


#endif //CRUST_FMT_HPP
