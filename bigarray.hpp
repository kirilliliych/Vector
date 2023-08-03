#ifndef BIG_ARRAY_HPP
#define BIG_ARRAY_HPP


#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include "specialvalues.hpp"


class BigArray
{
protected:
//---------------------------------------------------------------------------------
    BigArray(uint64_t capacity = DEFAULT_CAPACITY)
      : raw_data_(new char[capacity]),
        end_(raw_data_ + capacity)
    {
        while (raw_data_ == nullptr)
        {
            if (capacity == 1)
            {
                std::cerr << "ERROR: total memory exhaustion" << std::endl;
                assert(0);
            }

            capacity >>= 1;
            raw_data_ = new char[capacity];
        }
    }

    BigArray(const BigArray &other) = delete;
    BigArray(BigArray &&other)      = delete;

    BigArray &operator =(const BigArray &other) = delete;
    BigArray &operator =(BigArray &&other)      = delete;

    ~BigArray()
    {
        delete [] raw_data_;
        raw_data_ = const_cast<char *> (DESTR_PTR);
    }
//---------------------------------------------------------------------------------
protected:
//-----------------------------------Variables-------------------------------------
    static const uint64_t DEFAULT_CAPACITY = (1lu << 14);

    char *raw_data_ = const_cast<char *> (UNINIT_PTR);
    char *end_ = raw_data_ + DEFAULT_CAPACITY;            // NOTE: end_ points AFTER the ending of data
};

#endif
