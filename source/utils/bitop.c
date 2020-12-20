#include"include/utils/bitop.h"

// extern "C"
#include"cbegin.h"

extern size_t bitop_index_of_set(size_t bmp){
    // return 0; TODO
}

extern bool bitop_bmp_get(const size_t * bmp, size_t index){
    size_t i_word = index / sizeof(size_t);
    size_t i_bit  = index % sizeof(size_t);
    size_t i_mask = (size_t)1 << i_bit;
    return (bmp[i_word] & i_mask) != 0;
}

extern void bitop_bmp_set(size_t * bmp, size_t index){
    size_t i_word = index / sizeof(size_t);
    size_t i_bit  = index % sizeof(size_t);
    size_t i_mask = (size_t)1 << i_bit;
    bmp[i_word]  |= i_mask;
}

extern void bitop_bmp_reset(size_t * bmp, size_t index){
    size_t i_word = index / sizeof(size_t);
    size_t i_bit  = index % sizeof(size_t);
    size_t i_mask = (size_t)1 << i_bit;
    bmp[i_word]  &= ~i_mask;
}

extern bool bitop_bmp_test_and_reset(size_t * bmp, size_t index){
    size_t i_word = index / sizeof(size_t);
    size_t i_bit  = index % sizeof(size_t);
    size_t i_mask = (size_t)1 << i_bit;
    bool   r      = (bmp[i_word] & i_mask) != 0;
    bmp[i_word]  &= ~i_mask;
    return r;
}

// extern "C" end
#include"cend.h"
