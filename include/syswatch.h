#ifndef HW_SYSWATCH_H
#define HW_SYSWATCH_H

#include"include/sysconfig.h"

// extern "C"
#include"cbegin.h"

typedef void (* syswatch_stream_invoke)(void * data, size_t bytes);

// data group
typedef enum _sysdata_t{
    I_SYSCPU,
    I_SYSMEM,
    I_SYSMAX,
} sysdata_t;

enum _syscpu_fetch_it{
    I_SYSCPU_SOCKET_NUM,
    I_SYSCPU_CORE_NUM,
    I_SYSCPU_LOAD1,
    I_SYSCPU_LOAD5,
    I_SYSCPU_LOAD15,
    I_SYSCPU_ONLINEX,
    I_SYSCPU_USAGEX,
};

typedef enum _syscpu_fetch_t{
    SYSCPU_SOCKET_NUM   = 1 << I_SYSCPU_SOCKET_NUM,
    SYSCPU_CORE_NUM     = 1 << I_SYSCPU_CORE_NUM,
    SYSCPU_LOAD1        = 1 << I_SYSCPU_LOAD1,
    SYSCPU_LOAD5        = 1 << I_SYSCPU_LOAD5,
    SYSCPU_LOAD15       = 1 << I_SYSCPU_LOAD15,
    SYSCPU_ONLINEX      = 1 << I_SYSCPU_ONLINEX,
    SYSCPU_USAGEX       = 1 << I_SYSCPU_USAGEX,
} syscpu_fetch_t;

// level 1
enum _sysmem_fetch_it{
    I_SYSMEM_TOTAL,
    I_SYSMEM_USED,
    I_SYSMEM_USAGE,
    I_SYSMEM_FREE,
    I_SYSMEM_SHARED,
    I_SYSMEM_CACHE,
    I_SYSMEM_SWAP_TOTAL,
    I_SYSMEM_SWAP_USED,
    I_SYSMEM_SWAP_USAGE,
    I_SYSMEM_HUGEPAGES_TOTAL,
    I_SYSMEM_HUGEPAGES_FREE,
    I_SYSMEM_HUGEPAGES_RSVD,
    I_SYSMEM_HUGEPAGES_SURP,
    I_SYSMEM_HUGEPAGES_USAGE,
    I_SYSMEM_NUMAX,
};

enum _sysnuma_fetch_it{
    I_SYSMEM_NUMAX_TOTAL,
    I_SYSMEM_NUMAX_USED,
    I_SYSMEM_NUMAX_USAGE,
    I_SYSMEM_NUMAX_FREE,
};

typedef enum _sysmem_fetch_t{
    SYSMEM_TOTAL                = 1 << I_SYSMEM_TOTAL,
    SYSMEM_USED                 = 1 << I_SYSMEM_USED,
    SYSMEM_USAGE                = 1 << I_SYSMEM_USAGE,
    SYSMEM_FREE                 = 1 << I_SYSMEM_FREE,
    SYSMEM_SHARED               = 1 << I_SYSMEM_SHARED,
    SYSMEM_CACHE                = 1 << I_SYSMEM_CACHE,
    SYSMEM_SWAP_TOTAL           = 1 << I_SYSMEM_SWAP_TOTAL,
    SYSMEM_SWAP_USED            = 1 << I_SYSMEM_SWAP_USED,
    SYSMEM_SWAP_USAGE           = 1 << I_SYSMEM_SWAP_USAGE,
    SYSMEM_HUGEPAGES_TOTAL      = 1 << I_SYSMEM_HUGEPAGES_TOTAL,
    SYSMEM_HUGEPAGES_FREE       = 1 << I_SYSMEM_HUGEPAGES_FREE,
    SYSMEM_HUGEPAGES_RSVD       = 1 << I_SYSMEM_HUGEPAGES_RSVD,
    SYSMEM_HUGEPAGES_SURP       = 1 << I_SYSMEM_HUGEPAGES_SURP,
    SYSMEM_HUGEPAGES_USAGE      = 1 << I_SYSMEM_HUGEPAGES_USAGE,
    SYSMEM_NUMAX                = 1 << I_SYSMEM_NUMAX,
} sysmem_fetch_t;

// level 2
typedef enum _sysnuma_fetch_t{
    I_SYSMEM_NUMAX_TOTAL        = 1 << I_SYSMEM_NUMAX_TOTAL,
    I_SYSMEM_NUMAX_USED         = 1 << I_SYSMEM_NUMAX_USED,
    I_SYSMEM_NUMAX_USAGE        = 1 << I_SYSMEM_NUMAX_USAGE,
    I_SYSMEM_NUMAX_FREE         = 1 << I_SYSMEM_NUMAX_FREE,
} _sysnuma_fetch_t;

typedef enum _sysio_fetch_it{
    I_SYSIO_RRQMX,
    I_SYSIO_WRQMX,
    I_SYSIO_RX,
    I_SYSIO_WX,
    I_SYSIO_RKBX,
    I_SYSIO_WKBX,
    I_SYSIO_AVGRG_SZX,
    I_SYSIO_AVGQU_SZX,
    I_SYSIO_SVCTMX,
    I_SYSIO_UTILX,
    I_SYSIO_AWAITX,
} sysio_fetch_it;

typedef enum _sysio_fetch_t{
    SYSIO_RRQMX                 = 1 << I_SYSIO_RRQMX,
    SYSIO_WRQMX                 = 1 << I_SYSIO_WRQMX,
    SYSIO_RX                    = 1 << I_SYSIO_RX,
    SYSIO_WX                    = 1 << I_SYSIO_WX,
    SYSIO_RKBX                  = 1 << I_SYSIO_RKBX,
    SYSIO_WKBX                  = 1 << I_SYSIO_WKBX,
    SYSIO_AVGRG_SZX             = 1 << I_SYSIO_AVGRG_SZX,
    SYSIO_AVGQU_SZX             = 1 << I_SYSIO_AVGQU_SZX,
    SYSIO_SVCTMX                = 1 << I_SYSIO_SVCTMX,
    SYSIO_UTILX                 = 1 << I_SYSIO_UTILX,
    SYSIO_AWAITX                = 1 << I_SYSIO_AWAITX,
} sysio_fetch_t;

typedef struct _syscpu_load{
    int64_t             total_time;
    int64_t             idle_time;
} syscpu_load;

typedef struct _syscpu_fetch_guide{
    // syscpu_fetch_t
    uint32_t            mask;
    size_t              mask_bmp_cpu_online[SYSW_CPU_BMP_SIZE];
    size_t              mask_bmp_cpu_usage[SYSW_CPU_BMP_SIZE + 1/* for total load*/];
    syscpu_load         list_last_load[SYSW_MAX_CPU_NUM + 1/* for total load*/];
} syscpu_fetch_guide;

typedef struct _syscpu_data_template{
    uint32_t            socket_num;
    uint32_t            core_num;
    float               load1;
    float               load5;
    float               load15;
    bool                onlinex;
    float               usagex;

    // addition field
    int32_t             i_cpu;
} syscpu_data_template;


typedef struct _sysmem_fetch_guide{
    // sysmem_fetch_t
    uint32_t            mask;
    size_t              mask_bmp_numa_total [SYSW_MAX_CPU_NUM];
    size_t              mask_bmp_numa_used  [SYSW_MAX_CPU_NUM];
    size_t              mask_bmp_numa_usage [SYSW_MAX_CPU_NUM];
    size_t              mask_bmp_numa_free  [SYSW_MAX_CPU_NUM];
} sysmem_fetch_guide;

typedef struct _sysmem_data_template{
    uint32_t            total_mb;
    uint32_t            used_mb;
    float               usage;                  // float ?
    uint32_t            free_mb;
    uint32_t            shared_mb;              // MB ?
    uint32_t            cache_mb;               // MB ?
    uint32_t            swap_total_mb;          // MB ?
    uint32_t            swap_free_mb;           // MB ?
    float               swap_usage;             // float ??
    uint32_t            huage_page_total;       // MB ?
    uint32_t            huage_page_free;
    uint32_t            huage_page_rsvd;
    uint32_t            huage_page_surp;
    float               huage_page_usage;

    float               numa_total;             // unit ?
    float               numa_used;
    float               numa_usage;
    float               numa_free;

    uint16_t            i_numa;
    uint16_t            mask_numa;
} sysmem_data_template;

typedef struct _sysio_stat{
    // keep int64_t as the field type, 
    // it is assumed this structure can convert to int64_t array
    int64_t             read_completed;
    int64_t             read_merged;
    int64_t             read_sectors;
    int64_t             read_ms;
    int64_t             write_completed;
    int64_t             write_merged;
    int64_t             write_sectors;
    int64_t             write_ms;
    int64_t             cur_queue_size;
    int64_t             io_ms;
} sysio_stat;


typedef struct _sysio_statex{
    // there use redundancy field to keep context 
    // for fit different sampling period of 'sysio_field'
    int64_t             read_completed_for_r;
    int64_t             read_completed_for_scvtm;
    int64_t             read_completed_for_await;
    int64_t             read_merged_for_rrqm;
    int64_t             read_merged_for_avgrg_sz;
    int64_t             read_sectors_for_rkB;
    int64_t             read_sectors_for_avgrg_sz;
    int64_t             read_ms_for_await;
    int64_t             write_completed_for_w;
    int64_t             write_completed_for_scvtm;
    int64_t             write_completed_for_await;
    int64_t             write_merged_for_wrqm;
    int64_t             write_merged_for_avgrg_sz;
    int64_t             write_sectors_for_wkB;
    int64_t             write_sectors_for_avgrg_sz;
    int64_t             write_ms_for_await;
    int64_t             cur_queue_size;
    int64_t             io_ms_for_svctm;
    int64_t             io_ms_for_util;
} sysio_statex;

// the unit is 'second'
typedef struct _sysio_period{
    float               s_rrqm;
    float               s_wrqm;
    float               s_r;
    float               s_w;
    float               s_rkB;
    float               s_wkB;
    float               s_avgrg_sz;
    float               s_avgqu_sz;
    float               s_svctm;
    float               s_util;
    float               s_await;
} sysio_period;

typedef struct _sysio_field{
    int32_t             id;                 // device major_id << 16 | minor_id

    // keep static, syswatch.c ref it by sizeof(name)
    char                name[48];           // name of disk
    float               rrqm;               // read request with merge per second
    float               wrqm;               // write request with merge per second
    float               r;                  // the total number of reads completed successfully
    float               w;                  // the total number of writes completed successfully
    float               rkB;                // the total number of KB read successfully
    float               wkB;                // the total number of KB write successfully
    float               avgrg_sz;           // the average request size
    float               avgqu_sz;           // the average queue size
    float               svctm;              // the average time(ms) spent doing I/O
    float               util;               // the average I/O usage
    float               await;              // the average I/O latency(ms)
    sysio_statex        stat;               // last io state
    sysio_period        period;             // sampling period of io state
} sysio_field;

typedef struct _sysio_data{
    size_t              disk_num;
    sysio_field         disk[SYSW_MAX_DISK_NUM];
} sysio_data;

typedef enum _sysnet_fetch_type{
    SYSNET_DUPLEX               = 0x00001,
    SYSNET_AUTONEG              = 0x00002,
    SYSNET_SPEED                = 0x00004,
    SYSNET_STATUS               = 0x00008,
    SYSNET_RXBYTES              = 0x00010,
    SYSNET_RX_PACKETS           = 0x00020,
    SYSNET_RX_DROPPED           = 0x00040,
    SYSNET_RX_DROPPED_PERCENT   = 0x00080,
    SYSNET_RX_ERRS              = 0x00100,
    SYSNET_RX_FIFO_ERRS         = 0x00200,
    SYSNET_RX_FRAME_ERRS        = 0x00400,
    SYSNET_RX_MCST              = 0x00800,
    SYSNET_TX_BYTES             = 0x01000,
    SYSNET_TX_PACKETS           = 0x02000,
    SYSNET_TX_DROPPED           = 0x04000,
    SYSNET_TX_DROPPED_PERCENT   = 0x08000,
    SYSNET_TX_ERRS              = 0x10000,
    SYSNET_TX_FIFO_ERRS         = 0x20000,
    SYSNET_TX_COLL              = 0x40000,
    SYSNET_TX_CARR              = 0x80000,
} sysnet_fetch_type;

typedef struct _sysnet_field{
    int32_t             duplex;
    bool                autoneg;
    int32_t             speed;
    bool                status;
    int64_t             rx_bytes;
    int64_t             rx_packets;
    int64_t             rx_dropped;
    float               rx_dropped_percent;
    int64_t             rx_errs;
    int64_t             rx_fifo_errs;
    int64_t             rx_frame_errs;
    float               rx_mcst;
    int64_t             tx_bytes;
    int64_t             tx_packets;
    int64_t             tx_dropped;
    float               tx_dropped_percent;
    int64_t             tx_errs;
    int64_t             tx_fifo_errs;
    float               tx_coll;
    float               tx_carr;
} sysnet_field;

typedef struct _sysnet_data{
    size_t              eth_num;
    sysnet_field        eth[SYSW_MAX_ETH_NUM];
} sysnet_data;

// sampling control block
typedef struct _sys_scb{
    // use 64bit data type may not overflow for a very long time
    const char *        name;
    uint32_t            time_interval;
    uint32_t            i_in_heap;
    uint16_t            i_group;
    size_t              i_data;
    uint64_t            wakeup_time;
} sys_scb;

extern void syswatch_init();
extern void syswatch_tx_cpuinfo(syscpu_fetch_guide * guide, syswatch_stream_invoke stream);
extern void syswatch_tx_meminfo(sysmem_fetch_guide * guide, syswatch_stream_invoke stream);
extern void syswatch_get_ioinfo(sysio_data * info/*TODO: , sysio_fetch_type mask*/);
extern void syswatch_get_netinfo(sysnet_data * info/*TODO: , sysnet_fetch_type mask*/);

// end extern "C" 
#include"cend.h"
#endif
