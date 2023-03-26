#ifndef MY_REMOVE_REFERENCE_HPP
#define MY_REMOVE_REFERENCE_HPP


template<typename T>
struct my_remove_reference
{
    typedef T type;
};

template<typename T>
struct my_remove_reference<T &>
{
    typedef T type;
};

template<typename T>
struct my_remove_reference<T &&>
{
    typedef T type;
};


#endif
