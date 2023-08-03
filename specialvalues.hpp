#ifndef SPECIAL_VALUES_HPP
#define SPECIAL_VALUES_HPP


#include <iostream>


static const char *UNINIT_PTR           = reinterpret_cast<const char *> (0xDEADBEEF);
static const char *DESTR_PTR            = reinterpret_cast<const char *> (0xBAADF00D);
static const char *INVALID_PTR          = reinterpret_cast<const char *> (0xDEADDEAD);
static const char *MOVED_REMAINDERS_PTR = reinterpret_cast<const char *> (0xFEEDCAFE);
static const size_t POISONED_UINT64_T   = 0xAB0BAC0C;


#endif