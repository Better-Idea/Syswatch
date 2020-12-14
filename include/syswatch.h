#ifndef HW_SYSWATCH_H
#define HW_SYSWATCH_H

#include"include/sysconfig.h"

// extern "C"
#include"cbegin.h"

// data group
typedef enum _sysdata_type{
    I_SYSCPU,
    I_SYSMEM,
} sysdata_type;

typedef enum _syscpu_fetch_type{
    SYSCPU_SOCKET_NUM   = 0x01,
    SYSCPU_CORE_NUM     = 0x02,
    SYSCPU_LOAD         = 0x04,
    SYSCPU_ONLINE       = 0x08,
    SYSCPU_USAGE        = 0x10,
} syscpu_fetch_type;

typedef struct _syscpu_load{
    int64_t             total_time;
    int64_t             idle_time;
} syscpu_load;

typedef struct _syscpu_data{
    int32_t             socket_num;
    int32_t             core_num;
    float               load1;
    float               load5;
    float               load15;

    // keep static size, the memset will zeros it
    size_t              list_cpu_online_bmp[SYSW_CPU_BMP_SIZE];

    // the 1st item is total_usage, the follows are cpu0, cpu1 ...
    float               list_cpu_usage[SYSW_MAX_CPU_NUM + 1/* for total usage*/];
    syscpu_load         list_last_load[SYSW_MAX_CPU_NUM + 1/* for total load*/];
} syscpu_data;

typedef struct _sysnuma_data{
    float               total;
    float               used;
    float               usage;
    float               free;
} sysnuma_data;

typedef struct _sysmem_data{
    int32_t             total_mb;
    int32_t             used_mb;
    int32_t             usage;              // ??????????? float ?
    int32_t             free_mb;
    int32_t             shared_mb;          // MB ?
    int32_t             cache_mb;           // MB ?
    int32_t             swap_total_mb;      // MB ?
    int32_t             swap_free_mb;       // MB ?
    int32_t             swap_usage;         // float
    int32_t             huage_page_total;   // MB ?
    int32_t             huage_page_free;
    int32_t             huage_page_rsvd;
    int32_t             huage_page_surp;
    int32_t             huage_page_usage;
    int32_t             numa_num;
    sysnuma_data        list_numa[SYSW_MAX_NUMA_NUM];
} sysmem_data;

typedef enum _sysio_fetch_type{
    SYSIO_RRQM          = 0x0001,
    SYSIO_WRQM          = 0x0002,
    SYSIO_R             = 0x0004,
    SYSIO_W             = 0x0008,
    SYSIO_RKB           = 0x0010,
    SYSIO_WKB           = 0x0020,
    SYSIO_AVGRG_SZ      = 0x0040,
    SYSIO_AVGQU_SZ      = 0x0080,
    SYSIO_SVCTM         = 0x0100,
    SYSIO_UTIL          = 0x0200,
    SYSIO_AWAIT         = 0x0400,
} sysio_fetch_type;

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
    uint64_t            time_interval;
    const char *        name;
    uint32_t            i_in_heap;
    uint16_t            i_group;
    uint16_t            i_data;
} sys_scb;

extern void syswatch_init();
extern void syswatch_get_cpuinfo(syscpu_data * info, syscpu_fetch_type mask);
extern void syswatch_get_meminfo(sysmem_data * info/*TODO: , sysmem_fetch_type mask*/);
extern void syswatch_get_ioinfo(sysio_data * info/*TODO: , sysio_fetch_type mask*/);
extern void syswatch_get_netinfo(sysnet_data * info/*TODO: , sysnet_fetch_type mask*/);

// end extern "C" 
#include"cend.h"
#endif
