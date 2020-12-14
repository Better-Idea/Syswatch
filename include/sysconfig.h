#ifndef HW_SYSCONFIG_H
#define HW_SYSCONFIG_H

#include<stdbool.h>
#include<stddef.h>
#include<stdint.h>

#define SYSW_MAX_CPU_NUM                256
#define SYSW_MAX_NUMA_NUM               32
#define SYSW_MAX_DISK_NUM               128
#define SYSW_MAX_ETH_NUM                32

#define SYSW_CPU_BMP_SIZE               \
    ((SYSW_MAX_CPU_NUM / (sizeof(size_t) * 8)) + SYSW_MAX_CPU_NUM % (sizeof(size_t) * 8) != 0)


#define SYSWATCH_DEFAULT_PATH_PERIOD    "syswatch.rate"

#endif
