#include"include/syswatch.h"

// extern "C"
#include"cbegin.h"

void syswatch_heap_push(sys_scb ** list, size_t length, sys_scb * value){
    size_t    i         = length;
    size_t    ii;
    sys_scb * parent;

    while(i > 0){
        parent          = list[
            ii          = (i - 1) >> 1
        ];

        if (parent->time_interval < value->time_interval){
            break;
        }

        list[i]         = (parent);
        parent->i_in_heap
                        = (uint32_t)i;
        i               = (ii);
    }

    list[i]             = (value);
    value->i_in_heap    = (uint32_t)i;
}


sys_scb * syswatch_heap_pop(sys_scb ** list, size_t length, size_t index, sys_scb * insert){
    sys_scb * left;
    sys_scb * right;
    sys_scb * select;
    sys_scb * wanted    = (list[index]);
    size_t    i         = (index);
    size_t    i_left    = (index << 1) + 1;

    while(i_left + 1 < length) {
        left            = list[i_left];
        right           = list[i_left + 1];

        if (left->time_interval > right->time_interval){
            select      = right;
            i_left     += 1;
        }
        else{
            select      = left;
        }

        if (select->time_interval >= insert->time_interval){
            break;
        }

        list[i]         = (select);
        select->i_in_heap
                        = (i);
        i               = (i_left);
        i_left          = (i_left << 1) + 1;
    }

    list[i]             = (insert);
    insert->i_in_heap   = (i);
    return wanted;
}

#include"cend.h"
