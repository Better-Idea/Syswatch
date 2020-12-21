#ifndef HW_SYSWATCH_H
#define HW_SYSWATCH_H

#include"include/sysconfig.h"

// extern "C"
#include"cbegin.h"

typedef void (* syswatch_stream_invoke)(void * data, size_t bytes);
typedef void (* syswatch_guide_invoke)(void * guide, size_t i, syswatch_stream_invoke stream);
typedef void (* syswatch_tx_invoke)(void * guide, syswatch_stream_invoke stream);

// sampling control block
typedef struct _sys_scb{
    const char *        name;
    uint32_t            ms_period;

    // 'i' prefix meanings index
    uint32_t            i_group;
    uint32_t            i_mask;
    uint32_t            i_addition;
    uint32_t            i_in_heap;

    // use 64bit data type may not overflow for a very long time
    uint64_t            wakeup_time;
} sys_scb;

// data group
typedef enum _sysdata_group_t{
    I_SYSCPU,
    I_SYSMEM,
    I_SYSIO,
    I_SYSNET,
    I_SYSFSC,       // file system change
    I_SYSFSP,       // file system partition
    I_SYSMAX,
} sysdata_group_t;

enum _syscpu_fetch_it{
    I_SYSCPU_SOCKET_NUM,
    I_SYSCPU_CORE_NUM,
    I_SYSCPU_LOAD1,
    I_SYSCPU_LOAD5,
    I_SYSCPU_LOAD15,
    B_SYSCPU_XDATA,             // 'b' prefix meanings base
    I_SYSCPU_ONLINEX            = B_SYSCPU_XDATA,
    I_SYSCPU_USAGEX,
};

typedef enum _syscpu_fetch_t{
    SYSCPU_SOCKET_NUM           = 1 << I_SYSCPU_SOCKET_NUM,
    SYSCPU_CORE_NUM             = 1 << I_SYSCPU_CORE_NUM,
    SYSCPU_LOAD1                = 1 << I_SYSCPU_LOAD1,
    SYSCPU_LOAD5                = 1 << I_SYSCPU_LOAD5,
    SYSCPU_LOAD15               = 1 << I_SYSCPU_LOAD15,
    SYSCPU_ONLINEX              = 1 << I_SYSCPU_ONLINEX,
    SYSCPU_USAGEX               = 1 << I_SYSCPU_USAGEX,
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
    B_SYSMEM_NUMA,              // base of numa
    I_SYSMEM_NUMA_TOTALX        = B_SYSMEM_NUMA,
    I_SYSMEM_NUMA_USEDX,
    I_SYSMEM_NUMA_USAGEX,
    I_SYSMEM_NUMA_FREEX,
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
    SYSMEM_NUMA_TOTALX          = 1 << I_SYSMEM_NUMA_TOTALX,
    SYSMEM_NUMA_USEDX           = 1 << I_SYSMEM_NUMA_USEDX,
    SYSMEM_NUMA_USAGEX          = 1 << I_SYSMEM_NUMA_USAGEX,
    SYSMEM_NUMA_FREEX           = 1 << I_SYSMEM_NUMA_FREEX,
    SYSMEM_NUMAX                =
        SYSMEM_NUMA_TOTALX  |
        SYSMEM_NUMA_USEDX   |
        SYSMEM_NUMA_USAGEX  |
        SYSMEM_NUMA_FREEX,
} sysmem_fetch_t;

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

typedef enum _sysnet_fetch_it{
    I_SYSNET_DUPLEXX,
    I_SYSNET_AUTONEGX,
    I_SYSNET_SPEEDX,
    I_SYSNET_STATUSX,
    I_SYSNET_RXBYTESX,
    I_SYSNET_RX_PACKETSX,
    I_SYSNET_RX_DROPPEDX,
    I_SYSNET_RX_DROPPED_PERCENTX,
    I_SYSNET_RX_ERRSX,
    I_SYSNET_RX_FIFO_ERRSX,
    I_SYSNET_RX_FRAME_ERRSX,
    I_SYSNET_RX_MCSTX,
    I_SYSNET_TX_BYTESX,
    I_SYSNET_TX_PACKETSX,
    I_SYSNET_TX_DROPPEDX,
    I_SYSNET_TX_DROPPED_PERCENTX,
    I_SYSNET_TX_ERRSX,
    I_SYSNET_TX_FIFO_ERRSX,
    I_SYSNET_TX_COLLX,
    I_SYSNET_TX_CARRX,
} sysnet_fetch_it;

typedef enum _sysnet_fetch_t{
    SYSNET_DUPLEXX              = 1 << I_SYSNET_DUPLEXX,
    SYSNET_AUTONEGX             = 1 << I_SYSNET_AUTONEGX,
    SYSNET_SPEEDX               = 1 << I_SYSNET_SPEEDX,
    SYSNET_STATUSX              = 1 << I_SYSNET_STATUSX,
    SYSNET_RXBYTESX             = 1 << I_SYSNET_RXBYTESX,
    SYSNET_RX_PACKETSX          = 1 << I_SYSNET_RX_PACKETSX,
    SYSNET_RX_DROPPEDX          = 1 << I_SYSNET_RX_DROPPEDX,
    SYSNET_RX_DROPPED_PERCENTX  = 1 << I_SYSNET_RX_DROPPED_PERCENTX,
    SYSNET_RX_ERRSX             = 1 << I_SYSNET_RX_ERRSX,
    SYSNET_RX_FIFO_ERRSX        = 1 << I_SYSNET_RX_FIFO_ERRSX,
    SYSNET_RX_FRAME_ERRSX       = 1 << I_SYSNET_RX_FRAME_ERRSX,
    SYSNET_RX_MCSTX             = 1 << I_SYSNET_RX_MCSTX,
    SYSNET_TX_BYTESX            = 1 << I_SYSNET_TX_BYTESX,
    SYSNET_TX_PACKETSX          = 1 << I_SYSNET_TX_PACKETSX,
    SYSNET_TX_DROPPEDX          = 1 << I_SYSNET_TX_DROPPEDX,
    SYSNET_TX_DROPPED_PERCENTX  = 1 << I_SYSNET_TX_DROPPED_PERCENTX,
    SYSNET_TX_ERRSX             = 1 << I_SYSNET_TX_ERRSX,
    SYSNET_TX_FIFO_ERRSX        = 1 << I_SYSNET_TX_FIFO_ERRSX,
    SYSNET_TX_COLLX             = 1 << I_SYSNET_TX_COLLX,
    SYSNET_TX_CARRX             = 1 << I_SYSNET_TX_CARRX,
} sysnet_fetch_t;

typedef enum _sysfsc_fetch_it{
    I_SYSFS_FILE_DIR_CHANGE,
    B_SYSFS_PART,
    I_SYSFS_PART_RWX            = B_SYSFS_PART,
    I_SYSFS_PART_INODE_TOTALX,
    I_SYSFS_PART_INODE_USEDX,
    I_SYSFS_PART_INODE_USAGEX,
    I_SYSFS_PART_BYTES_TOTALX,
    I_SYSFS_PART_BYTES_USEDX,
    I_SYSFS_PART_BYTES_USAGEX,
} sysfsc_fetch_it;

typedef enum _sysfsc_fetch_t{
    SYSFS_FILE_DIR_CHANGE       = 1 << I_SYSFS_FILE_DIR_CHANGE,
    SYSFS_PART_RWX              = 1 << I_SYSFS_PART_RWX,
    SYSFS_PART_INODE_TOTALX     = 1 << I_SYSFS_PART_INODE_TOTALX,
    SYSFS_PART_INODE_USEDX      = 1 << I_SYSFS_PART_INODE_USEDX,
    SYSFS_PART_INODE_USAGEX     = 1 << I_SYSFS_PART_INODE_USAGEX,
    SYSFS_PART_BYTES_TOTALX     = 1 << I_SYSFS_PART_BYTES_TOTALX,
    SYSFS_PART_BYTES_USEDX      = 1 << I_SYSFS_PART_BYTES_USEDX,
    SYSFS_PART_BYTES_USAGEX     = 1 << I_SYSFS_PART_BYTES_USAGEX,
    SYSFS_PARTX                 =
        SYSFS_PART_RWX          |
        SYSFS_PART_INODE_TOTALX |
        SYSFS_PART_INODE_USEDX  |
        SYSFS_PART_INODE_USAGEX |
        SYSFS_PART_BYTES_TOTALX |
        SYSFS_PART_BYTES_USEDX  |
        SYSFS_PART_BYTES_USAGEX,
} sysfsc_fetch_t;

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

typedef struct _sysio_fetch_guide_item{
    // sysio_fetch_t
    uint32_t            mask;
    const char *        path_dev;
    sysio_statex        stat;
    sysio_period        period;
} sysio_fetch_guide_item, sysio_fgi;

typedef struct _sysio_fetch_guide{
    // this field meansing:
    // needed = at_least_one_bit_set @ (mask_bmp_disk)
    bool                needed;
    size_t              disk_num;
    size_t              mask_bmp_disk[SYSW_DISK_BMP_SIZE];
    sysio_fgi           disk[SYSW_MAX_DISK_NUM];
} sysio_fetch_guide;

typedef struct _sysio_data_template{
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

    // addition
    uint16_t            i_master;
    uint16_t            i_slaver;
} sysio_data_template;

typedef struct _sysnet_data_template{
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

    // addition
    uint16_t            i_master;
    uint16_t            i_slaver;
} sysnet_data_template;

typedef struct _sysnet_fetch_guide_item{
    uint32_t            mask;
    const char *        name_dev;

    // addition
    uint16_t            i_master;
    uint16_t            i_slaver;
} sysnet_fetch_guide_item, sysnet_fgi;

typedef struct _sysnet_fetch_guide{
    // this field meansing:
    // needed = at_least_one_bit_set @ (mask_bmp_eth)
    bool                needed;
    size_t              eth_num;
    size_t              mask_bmp_eth[SYSW_ETH_BMP_SIZE];
    sysio_fgi           eth[SYSW_ETH_BMP_SIZE];
} sysnet_fetch_guide;

typedef struct _syspro_fetch_guide{

} syspro_fetch_guide;

typedef struct _syspro_data_template{
    uint32_t            max_proc_num;
    float               usage_proc;
    uint32_t            zombie_num;
    uint32_t            fd_used;
    float               fd_usage;
} syspro_data_template;

typedef struct _sysfsc_fetch_guide{
    // this field meansing:
    // needed = at_least_one_bit_set @ (mask_bmp_notify)
    bool                needed;
    size_t              notify_num;
    size_t   *          bmp_changed;        // dynamic alloced bmp
    size_t   *          mask_bmp_notify;    // dynamic alloced bmp
    int                 fd_notify;
} sysfsc_fetch_guide;

typedef struct _sysfsp_fetch_guide_item{
    uint32_t            mask;
    const char *        mount_point;
} sysfsp_fetch_guide_item, sysfsp_fgi;

typedef struct _sysfsp_fetch_guide{
    // this field meansing:
    // needed = at_least_one_bit_set @ (mask_bmp_disk)
    bool                needed;
    size_t              disk_num;
    size_t              mask_bmp_disk[SYSW_DISK_BMP_SIZE];
    sysfsp_fgi          disk[SYSW_DISK_BMP_SIZE];
} sysfsp_fetch_guide;

// combine sysfsc and sysfsp data template
typedef struct _sysfsx_data_template{
    bool                changed;
    bool                read_write;
    uint32_t            inode_total;
    uint32_t            inode_used;
    float               inode_usage;
    uint64_t            bytes_total;
    uint64_t            bytes_used;
    float               bytes_usage;

    // addition
    uint32_t            wd;
} sysfsx_data_template;

extern void syswatch_init();
extern void syswatch_tx_cpuinfo(syscpu_fetch_guide * guide, syswatch_stream_invoke stream);
extern void syswatch_tx_meminfo(sysmem_fetch_guide * guide, syswatch_stream_invoke stream);
extern void syswatch_tx_ioinfo (sysio_fetch_guide  * guide, syswatch_stream_invoke stream);
extern void syswatch_tx_netinfo(sysnet_fetch_guide * guide, syswatch_stream_invoke stream);
// TODO:
extern void syswatch_tx_proinfo();
extern void syswatch_tx_fscinfo(sysfsc_fetch_guide * guide, syswatch_stream_invoke stream);
extern void syswatch_tx_fspinfo(sysfsp_fetch_guide * guide, syswatch_stream_invoke stream);

// end extern "C" 
#include"cend.h"
#endif
