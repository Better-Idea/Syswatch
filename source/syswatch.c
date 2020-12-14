#include<dirent.h>
#include<fcntl.h>
#include<linux/ethtool.h>
#include<linux/sockios.h>
#include<linux/if.h>
#include<netinet/in.h>
#include<stdbool.h>
#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/ioctl.h>
#include<string.h>
#include<unistd.h>

#include"include/sysconfig.h"
#include"include/syswatch.h"
#include"include/utils/bitop.h"

// extern "C"
#include"cbegin.h"

extern sys_scb      list_sbc[];
extern sys_scb *    list_sbc_ptr[];
extern const size_t list_sbc_num;

enum{
    I_CPU_HEADER            = INT32_MIN,
    SECTOR_BYTES            = 512,
};

static int syswatch_cmp_for_sbc(const void * left, const void * right){
    const sys_scb ** l = (const sys_scb **)left;
    const sys_scb ** r = (const sys_scb **)right;
    return strcmp(l[0]->name, r[0]->name);
}

extern void syswatch_init(){
    FILE *      fd_cfg;
    size_t      i           = 0;
    int32_t     period;
    sys_scb     word;
    sys_scb *   key         = & word;
    sys_scb **  find;
    int         argc;
    char        buf[128];
    char        name[128];

    for(; i < list_sbc_num; i++){
        list_sbc_ptr[i]     = & list_sbc[i];
    }

    // if there exist period configure file
    if (access(SYSWATCH_DEFAULT_PATH_PERIOD, F_OK) == 0){
        fd_cfg              = fopen(SYSWATCH_DEFAULT_PATH_PERIOD, "r");
        qsort(list_sbc_ptr, list_sbc_num, sizeof(list_sbc_ptr[0]), & syswatch_cmp_for_sbc);

        for(i = 0; i < list_sbc_num; i++){
            if (feof(fd_cfg)){
                // ERR maybe, not finished
                break;
            }

            fgets(buf, sizeof(buf), fd_cfg);
            argc            = sscanf(buf, "%s = %d", & name[0], & period);

            if (argc <= 1){
                // ERR
                continue;
            }

            key->name       = (name);
            find            = (sys_scb **)bsearch(
                & key, 
                list_sbc_ptr, 
                list_sbc_num, 
                sizeof(list_sbc_ptr[0]), 
                & syswatch_cmp_for_sbc
            );

            if (find == NULL){
                continue;
                // ERR maybe
            }

            find[0]->time_interval
                            = period;
        }

        fclose(fd_cfg);
    }
    else{
        i                   = 0;
    }

    // if configure file lose some values
    if (i < list_sbc_num){
        fd_cfg              = fopen(SYSWATCH_DEFAULT_PATH_PERIOD, "w");

        for(i = 0; i < list_sbc_num; i++){
            fprintf(fd_cfg, "%-30s = %d\n", list_sbc[i].name, list_sbc[i].time_interval);
        }

        fclose(fd_cfg);
    } 
}

static float syswatch_get_cpu_usage(syscpu_load load, syscpu_load last_load){
    // the idle time will be reset when the cpu was offline
    int64_t     d_idle      = load.idle_time - last_load.idle_time;
    int64_t     d_total     = load.total_time - last_load.total_time;
    int64_t     d_cost      = d_total - d_idle;

    if (d_cost <= 0 || d_idle <= 0){
        return 0.0f;
    }

    float       usage       = (float)d_cost / (float)d_total;
    return usage;
}

static syscpu_load syswatch_get_cpu_load(FILE * fd, int32_t * cpuid){
    enum{
        I_IDLE              = 3,
    };

    size_t      i           = 0;
    int64_t     t_cur       = 0;
    syscpu_load load        = { .idle_time = -1, .total_time = -1 };

    if (*cpuid == I_CPU_HEADER){
        *cpuid              = -1;
        i                   = fscanf(fd, "cpu"); // global
    }
    else{
        i                   = fscanf(fd, "cpu%d", cpuid); // single
    }

    if (i == -1){
        return load;
    }
    else{
        i                   = 0;
    }

    while(fscanf(fd, " %d", & t_cur) > 0){
        if (i == I_IDLE){
            load.idle_time  = t_cur;
        }

        i                  += 1;
        load.total_time    += t_cur;
    }
    return load;
}

extern void syswatch_get_cpuinfo(syscpu_data * info, syscpu_fetch_type mask){
    #define SYSW_PATH_CPU "/sys/devices/system/cpu"
    size_t  is_exist[SYSW_CPU_BMP_SIZE];
    size_t  prefix_length   = sizeof(SYSW_PATH_CPU) - 1 /*ignore '\0'*/;
    char    path_cpu[128]   = SYSW_PATH_CPU;
    char *  path_sub        = path_cpu + prefix_length;
    int32_t cpu_num         = 0;
    int32_t phy_id          = 0;
    int32_t socket_num      = 0;
    int32_t left_range      = 0;
    int32_t right_range     = 0;
    FILE *  fd              = NULL;
    #undef  SYSW_PATH_CPU

    memset(& is_exist, 0, sizeof(is_exist));

    if (mask & (SYSCPU_SOCKET_NUM | SYSCPU_CORE_NUM)) {
        for(; cpu_num < SYSW_MAX_CPU_NUM; cpu_num++, fclose(fd)){
            sprintf(path_sub, "/cpu%d/topology/physical_package_id", cpu_num);
            fd              = fopen(path_cpu, "r");

            if (fd == NULL){
                break;
            }
            if ((mask & SYSCPU_SOCKET_NUM) == 0){
                continue;
            }

            fscanf(fd, "%d", & phy_id);

            // if the 'phy_id' is unique
            if (bitop_bmp_get(is_exist, phy_id) == false){
                bitop_bmp_set(is_exist, phy_id);
                socket_num += 1;
            }
        }
    }

    if (mask & SYSCPU_SOCKET_NUM){
        info->socket_num    = socket_num;
    }
    if (mask & SYSCPU_CORE_NUM){
        info->core_num      = cpu_num;
    }
    if (mask & SYSCPU_LOAD){
        fd                  = fopen("/proc/loadavg", "r");
        fscanf(fd, "%f %f %f", & info->load1, & info->load5, & info->load15);
        fclose(fd);
    }
    if (mask & SYSCPU_ONLINE){
        memset(info->list_cpu_online_bmp, 0, sizeof(info->list_cpu_online_bmp));
        sprintf(path_sub, "/online");
        fd                  = fopen(path_cpu, "r");

        // at least one cpu online
        while(true){
            if (fscanf(fd, "%d", & left_range) <= 0){
                // ERR:TODO
            }
            if (fscanf(fd, "-%d", & right_range) == 1) while(left_range <= right_range){
                bitop_bmp_set(info->list_cpu_online_bmp, left_range/*index*/);
                left_range += 1;
            }
            else if (fscanf(fd, ",%d", & right_range) == 1){
                bitop_bmp_set(info->list_cpu_online_bmp, left_range/*index*/);
                bitop_bmp_set(info->list_cpu_online_bmp, right_range/*index*/);
            }
            if (fgetc(fd) != ','){
                fclose(fd);
                break;
            }
        }
    }
    if (mask & SYSCPU_USAGE){
        syscpu_load   cur;
        int32_t       i     = I_CPU_HEADER;
        float         usage = 0;
        float       * list_usage
                            = info->list_cpu_usage + 1;
        syscpu_load * old   = info->list_last_load + 1/* skip total cpu load, offset to cpu0 */;
        fd                  = fopen("/proc/stat", "r");

        while(true){
            cur             = syswatch_get_cpu_load(fd, & i);

            if (cur.total_time == -1){
                break;
            }

            // be sure use the 'i' as the index
            // becase the line of 'cpu*' will hidden when the corresponding cpu was offline
            usage           = syswatch_get_cpu_usage(cur, old[i]);
            old[i]          = cur;
            list_usage[i]   = usage;
        }

        fclose(fd);
    }
}

typedef struct _sysmem_token{
    const char * format;
    int64_t    * size;
    char         unit[8];
} sysmem_token;

static void syswatch_get_meminfo_core(
    FILE         *  fd, 
    sysmem_token *  token, 
    size_t          token_length, 
    int32_t      *  node_id){

    enum{
        MAX_SKIP_SIZE       = 128,
    };
    size_t         i        = 0;
    size_t         rest     = token_length;
    int            ret      = -1;
    char           line[MAX_SKIP_SIZE];
    sysmem_token * cur;

    while (feof(fd) == 0 && rest != 0){
        line[MAX_SKIP_SIZE - 1]
                            = 0x0; // guard
        fgets(line, sizeof(line), fd);

        if (line[MAX_SKIP_SIZE - 1] != 0x0){
            // ERR: overflow
        }

        for(i = 0; i < token_length; i++){
            cur             = & token[i];

            if (node_id == NULL){
                if (sscanf(line, cur->format, cur->size, cur->unit) <= 0){
                    continue;
                }
            }
            else{
                /*at least one, is the node_id*/
                if (sscanf(line, cur->format, node_id, cur->size, cur->unit) <= 1){
                    continue;
                }
            }
            if (strcmp(cur->unit, "kB") == 0 || strcmp(cur->unit, "KB") == 0){
                *cur->size    <<= 10;
            }
            else if (strcmp(cur->unit, "mB") == 0 || strcmp(cur->unit, "MB") == 0){
                *cur->size    <<= 20;
            }
            else if (cur->unit[0] != '\0'){
                // ERR:TODO
            }

            rest               -= 1;
            break;
        }
    }

    if (rest != 0){
        // ERR:TODO
    }
}

extern void syswatch_get_meminfo(sysmem_data * info/*TODO: , sysmem_fetch_type mask*/){
    // memory unit place holder
    #define SYSMEM_UNIT_PH  " %s"

    int64_t mem_total       = -1;
    int64_t mem_free        = -1;
    int64_t mem_shared      = -1;
    int64_t mem_buffer      = -1;
    int64_t mem_cache       = -1;
    int64_t swap_total      = -1;
    int64_t swap_free       = -1;
    sysmem_token 
            token[]         = {
        { " MemTotal:"  " %lld" SYSMEM_UNIT_PH, & mem_total , "" },
        { " MemFree:"   " %lld" SYSMEM_UNIT_PH, & mem_free  , "" },
        { " Buffers:"   " %lld" SYSMEM_UNIT_PH, & mem_buffer, "" },
        { " Cached:"    " %lld" SYSMEM_UNIT_PH, & mem_cache , "" },
        { " SwapTotal:" " %lld" SYSMEM_UNIT_PH, & swap_total, "" },
        { " SwapFree:"  " %lld" SYSMEM_UNIT_PH, & swap_free , "" },
        { " Shmem:"     " %lld" SYSMEM_UNIT_PH, & mem_shared, "" },
    };
    size_t  token_length    = sizeof(token) / sizeof(token[0]);
    FILE *  fd              = NULL;

    fd                      = fopen("/proc/meminfo", "r");
    syswatch_get_meminfo_core(fd, token, token_length, NULL);
    fclose(fd);
    // CHECK:TODO

    #define SYSMEM_SCALE    20/*1MB*/

    int64_t mem_used        = (mem_total - mem_free - mem_cache - mem_buffer);
    info->total_mb          = (int32_t)(mem_total   >> SYSMEM_SCALE);
    info->used_mb           = (int32_t)(mem_used    >> SYSMEM_SCALE);
    // info->usage             = ;
    info->free_mb           = (int32_t)(mem_free    >> SYSMEM_SCALE);
    info->shared_mb         = (int32_t)(mem_shared  >> SYSMEM_SCALE);
    info->cache_mb          = (int32_t)(mem_cache   >> SYSMEM_SCALE);
    info->swap_total_mb     = (int32_t)(swap_total  >> SYSMEM_SCALE);
    info->swap_free_mb      = (int32_t)(swap_free   >> SYSMEM_SCALE);
    // info->swap_usage        = ;
    // info->huage_page_total  = ;
    // info->huage_page_free   = ;
    // info->huage_page_rsvd   = ;
    // info->huage_page_surp   = ;
    // info->huage_page_usage  = ;

    #define SYSMEM_PATH_NODE "/sys/devices/system/node/node"

    char    path_node[64]   = SYSMEM_PATH_NODE;
    char *  path_sub        = path_node + sizeof(SYSMEM_PATH_NODE) - 1/*ignore '\0'*/;
    int32_t i_node          = 0;
    int32_t node_id         = 0;
    sysmem_token
            token_numa[]    = {
        { " Node %d MemTotal:"  " %lld" SYSMEM_UNIT_PH, & mem_total , "" },
        { " Node %d MemFree:"   " %lld" SYSMEM_UNIT_PH, & mem_free  , "" },
        { " Node %d MemUsed:"   " %lld" SYSMEM_UNIT_PH, & mem_used  , "" },
    };
    token_length            = sizeof(token_numa) / sizeof(token_numa[0]);

    while(true){
        sprintf(path_sub, "%d/meminfo", i_node);
        fd                  = fopen(path_node, "r");

        if (fd == NULL){
            break; // maybe ERR
        }

        syswatch_get_meminfo_core(fd, token_numa, token_length, & node_id);
        fclose(fd);

        if (node_id != i_node){
            // ERR
        }

        sysnuma_data * numa = & info->list_numa[node_id];
        // numa->total         = ;
        // numa->used          = ;
        // numa->usage         = ;
        // numa->free          = ;

        i_node             += 1;
    }

    #undef  SYSMEM_UNIT_PH
    #undef  SYSMEM_PATH_NODE
    #undef  SYSMEM_SCALE
}

static void syswatch_get_ioinfo_core(sysio_field * info, const sysio_stat * stat, sysio_fetch_type mask){
    sysio_statex * last                     = & info->stat;
    sysio_period * period                   = & info->period;

    if (mask & SYSIO_RRQM){
        info->rrqm                          = 
            (float)(stat->read_merged - last->read_merged_for_rrqm) / period->s_rrqm;
        last->read_merged_for_rrqm          = stat->read_merged;
    }
    if (mask & SYSIO_WRQM){
        info->wrqm                          = 
            (float)(stat->write_merged - last->write_merged_for_wrqm) / period->s_wrqm;
        last->write_merged_for_wrqm         = stat->write_merged;
    }
    if (mask & SYSIO_R){
        info->r                             = 
            (float)(stat->read_completed - last->read_completed_for_r) / period->s_r;
        last->read_completed_for_r          = stat->read_completed;
    }
    if (mask & SYSIO_W){
        info->w                             = 
            (float)(stat->write_completed - last->write_completed_for_w) / period->s_w;
        last->write_completed_for_w         = stat->write_completed;
    }
    if (mask & SYSIO_RKB){
        info->rkB                           = 
            (float)(stat->read_sectors - last->read_sectors_for_rkB) * SECTOR_BYTES / 1024/*KB*/ / period->s_rkB;
        last->read_sectors_for_rkB          = stat->read_sectors;
    }
    if (mask & SYSIO_WKB){
        info->wkB                           = 
            (float)(stat->write_sectors - last->write_sectors_for_wkB) * SECTOR_BYTES / 1024/*KB*/ / period->s_wkB;
        last->write_sectors_for_wkB = stat->write_sectors;
    }
    if (mask & SYSIO_AVGRG_SZ){ // average request size
        info->avgrg_sz                      = 
            (float)(
                stat->read_sectors + stat->write_sectors - 
                last->read_sectors_for_avgrg_sz - last->write_sectors_for_avgrg_sz) /
            (float)(
                stat->read_merged + stat->write_merged - 
                last->read_merged_for_avgrg_sz - last->write_merged_for_avgrg_sz) * 
            (float)(SECTOR_BYTES) / 1024/*KB*/;
        last->read_sectors_for_avgrg_sz     = stat->read_sectors;
        last->write_sectors_for_avgrg_sz    = stat->write_sectors;
        last->read_merged_for_avgrg_sz      = stat->read_merged;
        last->write_merged_for_avgrg_sz     = stat->write_merged;
    }
    if (mask & SYSIO_AVGQU_SZ){
        // TODO: ==========================================================================================
    }
    if (mask & SYSIO_SVCTM){
        info->svctm                         =
            (float)(stat->io_ms - last->io_ms_for_svctm) / 
            (float)(stat->read_completed + stat->write_completed - 
                last->read_completed_for_scvtm - last->write_completed_for_scvtm);
        last->io_ms_for_svctm               = stat->io_ms;
        last->read_completed_for_scvtm      = stat->read_completed;
        last->write_completed_for_scvtm     = stat->write_completed;
    }
    if (mask & SYSIO_UTIL){
        info->util                          =
            (float)(stat->io_ms - last->io_ms_for_util) / period->s_util;
        last->io_ms_for_util                = stat->io_ms;
    }
    if (mask & SYSIO_AWAIT){
        info->await                         =
            (float)(stat->read_ms + stat->write_ms - last->read_ms_for_await - last->write_ms_for_await) / 
            (float)(stat->read_completed + stat->write_completed - 
                last->read_completed_for_await - last->write_completed_for_await);
        last->read_ms_for_await             = stat->read_ms;
        last->write_ms_for_await            = stat->write_ms;
        last->read_completed_for_await      = stat->read_completed;
        last->write_completed_for_await     = stat->write_completed;
    }
}

extern void syswatch_get_ioinfo(sysio_data * info/*TODO: , sysio_fetch_type mask*/){
    #define SYSIO_PATH_DEV  "/sys/class/block/"
    char    path_dev[128]   = SYSIO_PATH_DEV;
    char  * path_sub        = path_dev + sizeof(SYSIO_PATH_DEV) - 1/*ignore '\0'*/;
    size_t  rest_size       = sizeof(path_dev) - sizeof(SYSIO_PATH_DEV);
    size_t  i               = 0;
    int32_t i_master        = 0;
    int32_t i_slaver        = 0;
    int32_t i_stat;
    DIR   * dev             = opendir(SYSIO_PATH_DEV);
    FILE  * fd;
    sysio_stat      stat;
    sysio_field   * block;
    int64_t       * io_stat = (int64_t *) & stat;
    struct dirent * sub;

    if (dev == NULL){
        // ERR
    }
    if (strcmp(readdir(dev)->d_name, ".") != 0){
        // ERR
    }
    if (strcmp(readdir(dev)->d_name, "..") != 0){
        // ERR
    }

    while(NULL != (sub = readdir(dev))){
        block               = & info->disk[i]; // TODO: index
        sprintf(path_sub, "%s/dev", sub->d_name);
        fd                  = fopen(path_dev, "r");

        // ERR
        if (fd == NULL){
            continue;
        }

        fscanf(fd, "%d:%d", & i_master, & i_slaver);
        fclose(fd);

        sprintf(path_sub, "%s/stat", sub->d_name);
        fd                  = fopen(path_dev, "r");
        block->id           = (i_master << 16) | i_slaver;
        i_stat              = (0);
        io_stat             = (int64_t *)& block->stat;

        if (fd == NULL){
            // ERR
        }

        while(i_stat < sizeof(sysio_stat) / sizeof(int64_t)){
            // ERR
            if (fscanf(fd, " %lld", & io_stat[i_stat]) <= 0){
                break;
            }

            i_stat         += 1;
        }

        fclose(fd);
        syswatch_get_ioinfo_core(block, & stat, // TODO:mask
            SYSIO_RRQM | SYSIO_WRQM | SYSIO_R | SYSIO_W | SYSIO_RKB | SYSIO_WKB | 
            SYSIO_AVGRG_SZ | SYSIO_AVGQU_SZ | SYSIO_SVCTM | SYSIO_UTIL | SYSIO_AWAIT
        );
    }
    closedir(dev);

    #undef  SYSIO_PATH_DEV
}

static int64_t syswatch_get_netinfo_fs(const char * path_fmt, const char * net_dev){
    int64_t value           = -1;
    char    path[128];
    FILE *  fd;
    sprintf(path, path_fmt, net_dev);
    fd                      = fopen(path, "r");

    if (fd == NULL){
        // ERR
    }

    fscanf(fd, "%lld", & value);
    fclose(fd);
    return value;
}

static void syswatch_get_netinfo_core(const char * eth_name, sysnet_field * field, sysnet_fetch_type mask){
    struct ifreq            req;
    struct ethtool_cmd      eth0;
    int                     fd;
    int tmp[32] = {0};
    
    strncpy(req.ifr_name, eth_name, sizeof(req.ifr_name));

    eth0.cmd                = ETHTOOL_GSET;
    req.ifr_data            = & eth0;
    fd                      = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (fd == -1){
        // ERR
        return;
    }

    if (ioctl(fd, SIOCETHTOOL, & req) != -1){
        if (mask & SYSNET_DUPLEX){
            field->duplex   = eth0.duplex;
        }
        if (mask & SYSNET_AUTONEG){
            field->autoneg  = eth0.autoneg;
        }
        if (mask & SYSNET_SPEED){
            field->speed    = ethtool_cmd_speed(& eth0);
        }
    }
    else{
        // ERR
    }

    close(fd);

    /* $ cat /proc/net/dev 
     *
     *  Inter-| Receive                                                   | Transmit
     *  face  | bytes    packets errs drop fifo frame compressed multicast| bytes    packets errs drop fifo colls carrier compressed
     *  ens33:  4384293  41111   0    0    0     0    0          0          771486   9619    0    0    0    0     0       0
     *  lo:     88       1       0    0    0     0    0          0          88       1       0    0    0    0     0       0
     *
     * maybe it will insert some field in the future, the order may not stable, 
     * resolution the title together perhaps right.
     * emmm... for esay to read, we use hard way to fetch the data.
     */

    #define SYSNET_PATH     "/sys/class/net/%s/"
    #define SYSNET_PATHX    "/sys/class/net/%s/statistics/"

    if (mask & SYSNET_STATUS){
        field->status       = syswatch_get_netinfo_fs(SYSNET_PATH  "carrier", eth_name) > 0;
    }
    if (mask & SYSNET_RXBYTES){
        field->rx_bytes     = syswatch_get_netinfo_fs(SYSNET_PATHX "rx_bytes", eth_name);
    }
    if (mask & SYSNET_RX_PACKETS){
        field->rx_packets   = syswatch_get_netinfo_fs(SYSNET_PATHX "rx_packets", eth_name);
    }
    if (mask & SYSNET_RX_DROPPED){
        field->rx_dropped   = syswatch_get_netinfo_fs(SYSNET_PATHX "rx_dropped", eth_name);
    }
    if (mask & SYSNET_RX_DROPPED_PERCENT){
        // TODO=========================================================
    }
    if (mask & SYSNET_RX_ERRS){
        field->rx_errs      = syswatch_get_netinfo_fs(SYSNET_PATHX "rx_errors", eth_name);
    }
    if (mask & SYSNET_RX_FIFO_ERRS){
        field->rx_fifo_errs = syswatch_get_netinfo_fs(SYSNET_PATHX "rx_fifo_errors", eth_name);
    }
    if (mask & SYSNET_RX_FRAME_ERRS){
        field->rx_frame_errs= syswatch_get_netinfo_fs(SYSNET_PATHX "rx_frame_errors", eth_name);
    }
    if (mask & SYSNET_RX_MCST){
        // field->rx_mcst          = 
    }
    if (mask & SYSNET_TX_BYTES){
        field->tx_bytes     = syswatch_get_netinfo_fs(SYSNET_PATHX "tx_bytes", eth_name);
    }
    if (mask & SYSNET_TX_PACKETS){
        field->tx_packets   = syswatch_get_netinfo_fs(SYSNET_PATHX "tx_packets", eth_name);
    }
    if (mask & SYSNET_TX_DROPPED){
        field->tx_dropped   = syswatch_get_netinfo_fs(SYSNET_PATHX "tx_dropped", eth_name);
    }
    if (mask & SYSNET_TX_DROPPED_PERCENT){
        // TODO=========================================================
    }
    if (mask & SYSNET_TX_ERRS){
        field->tx_errs      = syswatch_get_netinfo_fs(SYSNET_PATHX "tx_errors", eth_name);
    }
    if (mask & SYSNET_TX_FIFO_ERRS){
        field->tx_fifo_errs = syswatch_get_netinfo_fs(SYSNET_PATHX "tx_fifo_errors", eth_name);
    }
    if (mask & SYSNET_TX_COLL){
        field->tx_coll      = syswatch_get_netinfo_fs(SYSNET_PATHX "collisions", eth_name);
    }
    if (mask & SYSNET_TX_CARR){
        field->tx_carr      = syswatch_get_netinfo_fs(SYSNET_PATHX "tx_carrier_errors", eth_name);
    }

    #undef  SYSNET_PATH
    #undef  SYSNET_PATHX
}


// end extern "C" 
#include"cend.h"
