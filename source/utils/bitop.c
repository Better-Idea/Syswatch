#include"include/utils/bitop.h"

// extern "C"
#include"cbegin.h"

size_t bitop_index_of_set(size_t bmp){

}

bool bitop_bmp_get(const size_t * bmp, size_t index){
    size_t i_word = index / sizeof(size_t);
    size_t i_bit  = index % sizeof(size_t);
    size_t i_mask = (size_t)1 << i_bit;
    return (bmp[i_word] & i_mask) != 0;
}

void bitop_bmp_set(size_t * bmp, size_t index){
    size_t i_word = index / sizeof(size_t);
    size_t i_bit  = index % sizeof(size_t);
    size_t i_mask = (size_t)1 << i_bit;
    bmp[i_word]  |= i_mask;
}

// extern "C" end
#include"cend.h"
