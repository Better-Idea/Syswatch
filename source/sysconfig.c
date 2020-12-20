#include"include/sysconfig.h"
#include"include/syswatch.h"
#include<malloc.h>
// extern "C"
#include"cbegin.h"

sys_scb list_sbc[1024];

sys_scb * list_sbc_ptr[sizeof(list_sbc) / sizeof(list_sbc[0])];

const size_t list_sbc_num = sizeof(list_sbc) / sizeof(list_sbc[0]);

#include"cend.h"
