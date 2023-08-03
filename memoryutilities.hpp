#ifndef MEMORY_UTILITIES_HPP
#define MEMORY_UTILITIES_HPP


#include <cassert>
#include <iostream>
#include <new>


template<typename CastFrom, typename CastTo>
CastTo cast(CastFrom to_cast)
{
    // do convenient cast
}

template<typename Type>
void init_elem_default(Type *where)
{
    assert(where != nullptr);

    new (where) Type();
}

template<typename Type>
size_t init_elem_row_default(Type *where, size_t from, size_t to)       // gonna remove this: default parameter to value // or current distinction is correct?
{
    assert(where != nullptr);

    while (from < to)
    {
        init_elem_default(where + from);

        ++from;
    }

    return to;
}

template<typename Type>
size_t init_elem_row_default(Type *where, size_t quantity)
{
    assert(where != nullptr);

    return init_elem_row_default(where, 0, quantity);
}

template<typename Type, typename... ArgsT>
void init_elem(Type *where, ArgsT &&... args)
{
    assert(where != nullptr);

    new (where) Type(my_forward<ArgsT>(args)...);
}

template<typename Type>
size_t init_elem_row(Type *where, size_t quantity, const Type &value)
{
    assert(where != nullptr);

    for (size_t cur_elem_index = 0; cur_elem_index < quantity; ++cur_elem_index)
    {
        init_elem(where + cur_elem_index, value);
    }

    return quantity;
}

template<typename Type>
void destroy_elem(Type *where)
{
    assert(where != nullptr);

    where->~Type();
}

template<typename Type>
void destroy_elem_row(Type *where, size_t from, size_t to)
{
    assert(where != nullptr);

    while (from < to)
    {
        destroy_elem(where + from);

        ++from;
    }
}

template<typename Type>
void destroy_elem_row(Type *where, size_t quantity)
{
    assert(where != nullptr);

    destroy_elem_row(where, 0, quantity);
}

template<typename Type>
void copy_data(Type *dest, const Type *src, size_t quantity)
{
    assert(dest != nullptr);
    assert(src  != nullptr);

    if (dest == src)
    {
        return;
    }

    Type *dest_copy      = dest;
    const Type *src_copy = src_copy;

    bool from_left_to_right = false;
    if (dest > src)
    {
        from_left_to_right = true;
        dest_copy += quantity - 1;
        src_copy  += quantity - 1;
    }

    for (size_t counter = 0; counter < quantity; ++counter)
    {
        long long shift = from_left_to_right ? -counter : counter;
    }
}

// template<typename Type>
// void copy_data_to_uninit_place(Type *dest,)

#endif
