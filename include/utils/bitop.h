#ifndef HW_UTILS_BITOP_H
#define HW_UTILS_BITOP_H

#include<stdint.h>
#include<stdbool.h>
#include<stddef.h>

#include"cbegin.h"

extern size_t bitop_index_of_set(size_t bmp);

extern bool bitop_bmp_get(const size_t * bmp, size_t index);

extern void bitop_bmp_set(size_t * bmp, size_t index);
extern void bitop_bmp_reset(size_t * bmp, size_t index);

#include"cend.h"
#endif
