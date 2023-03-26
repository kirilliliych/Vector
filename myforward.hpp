#ifndef MY_FORWARD_HPP
#define MY_FORWARD_HPP


#include "myremovereference.hpp"


template<typename T>
T &&my_forward(typename my_remove_reference<T>::type &arg)
{
    return static_cast<T &&> (arg);
}

template<typename T>
T &&my_forward(typename my_remove_reference<T>::type &&arg)
{
    return static_cast<T &&>(arg);
}

#endif
