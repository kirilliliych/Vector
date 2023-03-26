#ifndef MY_MOVE_HPP
#define MY_MOVE_HPP


#include "myremovereference.hpp"


template<typename T>
typename my_remove_reference<T>::type &&my_move(T &&arg)
{
    return static_cast<typename my_remove_reference<T>::type &&> (arg);
}

#endif
