#ifndef _C11TEST_LAZY_HPP_
#define _C11TEST_LAZY_HPP_

#include "optional.hpp"

template<typename T>
class Lazy{
public:


private:
    Optional<T> value_;
};

#endif // _C11TEST_LAZY_HPP_
