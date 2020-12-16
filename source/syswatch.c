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

#define SYSDATA_END         -1

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

extern void syswatch_tx_cpuinfo(syscpu_fetch_guide * guide, syswatch_stream_invoke stream){
    if (guide->mask){
        stream(& guide->mask, sizeof(guide->mask));
    }
    else{
        return;
    }

    uint32_t    mask        = guide->mask;
    guide->mask             = 0; // reset

    #define SYSW_PATH_CPU "/sys/devices/system/cpu"
    typedef syscpu_data_template sdt_t;
    size_t  is_exist[SYSW_CPU_BMP_SIZE];
    size_t  cpu_online[SYSW_CPU_BMP_SIZE];
    size_t  prefix_length   = sizeof(SYSW_PATH_CPU) - 1 /*ignore '\0'*/;
    char    path_cpu[128]   = SYSW_PATH_CPU;
    char *  path_sub        = path_cpu + prefix_length;
    int32_t phy_id          = 0;
    int32_t left_range      = 0;
    int32_t right_range     = 0;
    FILE *  fd              = NULL;
    sdt_t   sdt             = {};
    #undef  SYSW_PATH_CPU

    if (mask & (SYSCPU_SOCKET_NUM | SYSCPU_CORE_NUM)) {
        memset(& is_exist, 0, sizeof(is_exist));

        for(; sdt.core_num < SYSW_MAX_CPU_NUM; sdt.core_num++, fclose(fd)){
            sprintf(path_sub, "/cpu%d/topology/physical_package_id", sdt.core_num);
            fd              = fopen(path_cpu, "r");

            if (fd == NULL){
                break;
            }
            if ((mask & SYSCPU_SOCKET_NUM) == 0){
                continue;
            }

            fscanf(fd, "%d", & phy_id);

            if (bitop_bmp_get(is_exist, phy_id)){
                continue;
            }

            // if the 'phy_id' is unique
            bitop_bmp_set(is_exist, phy_id);
            sdt.socket_num += 1;
        }
    }

    if (mask & SYSCPU_SOCKET_NUM){
        stream(& sdt.socket_num, sizeof(sdt.socket_num));
    }
    if (mask & SYSCPU_CORE_NUM){
        stream(& sdt.core_num, sizeof(sdt.core_num));
    }
    if (mask & (SYSCPU_LOAD1 | SYSCPU_LOAD5 | SYSCPU_LOAD15)){
        fd                  = fopen("/proc/loadavg", "r");
        fscanf(fd, "%f %f %f", & sdt.load1, & sdt.load5, & sdt.load15);
        fclose(fd);

        if (mask & SYSCPU_LOAD1){
            stream(& sdt.load1, sizeof(sdt.load1));
        }
        if (mask & SYSCPU_LOAD5){
            stream(& sdt.load5, sizeof(sdt.load5));
        }
        if (mask & SYSCPU_LOAD15){
            stream(& sdt.load15, sizeof(sdt.load15));
        }
    }
    if (mask & SYSCPU_ONLINEX){
        memset(cpu_online, 0, sizeof(cpu_online));
        sprintf(path_sub, "/online");
        fd                  = fopen(path_cpu, "r");

        // at least one cpu online
        while(true){
            if (fscanf(fd, "%d", & left_range) <= 0){
                // ERR:TODO
            }
            if (fscanf(fd, "-%d", & right_range) == 1) while(left_range <= right_range){
                bitop_bmp_set(cpu_online, left_range/*index*/);
                left_range += 1;
            }
            else if (fscanf(fd, ",%d", & right_range) == 1){
                bitop_bmp_set(cpu_online, left_range/*index*/);
                bitop_bmp_set(cpu_online, right_range/*index*/);
            }
            if (fgetc(fd) != ','){
                fclose(fd);
                break;
            }
        }

        for(sdt.i_cpu = 0; sdt.i_cpu < sizeof(guide->mask_bmp_cpu_online) * 8; sdt.i_cpu++){
            if (bitop_bmp_get(guide->mask_bmp_cpu_online, sdt.i_cpu)){
                bitop_bmp_get(cpu_online, sdt.i_cpu);
                stream(& sdt.i_cpu, sizeof(sdt.i_cpu));
                stream(& sdt.onlinex, sizeof(sdt.onlinex));
            }
        }

        sdt.i_cpu           = SYSDATA_END;
        stream(& sdt.i_cpu, sizeof(sdt.i_cpu));
    }
    if (mask & SYSCPU_USAGEX){
        syscpu_load   cur;
        syscpu_load * old   = guide->list_last_load + 1/* skip total cpu load, offset to cpu0 */;
        fd                  = fopen("/proc/stat", "r");
        sdt.i_cpu           = I_CPU_HEADER;

        while(true){
            cur             = syswatch_get_cpu_load(fd, & sdt.i_cpu);

            if (cur.total_time == -1){
                break;
            }
            if (bitop_bmp_get(guide->mask_bmp_cpu_usage, sdt.i_cpu) == false){
                continue;
            }

            // be sure use the 'i' as the index
            // becase the line of 'cpu*' will hidden when the corresponding cpu was offline
            sdt.usagex      = syswatch_get_cpu_usage(cur, old[sdt.i_cpu]);
            old[sdt.i_cpu]  = cur;
            stream(& sdt.i_cpu, sizeof(sdt.i_cpu));
            stream(& sdt.usagex, sizeof(sdt.usagex));
        }

        sdt.i_cpu           = SYSDATA_END;
        stream(& sdt.i_cpu, sizeof(sdt.i_cpu));
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

extern void syswatch_tx_meminfo(sysmem_fetch_guide * guide, syswatch_stream_invoke stream){
    if (guide->mask){
        stream(& guide->mask, sizeof(guide->mask));
    }
    else{
        return;
    }

    uint32_t    mask        = guide->mask;
    guide->mask             = 0; // reset

    // memory unit place holder
    #define SYSMEM_UNIT_PH  " %s"
    typedef sysmem_data_template sdt_t;
    sdt_t   sdt             = {};
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

    // CHECK:TODO
    fd                      = fopen("/proc/meminfo", "r");
    syswatch_get_meminfo_core(fd, token, token_length, NULL);
    fclose(fd);

    #define SYSMEM_SCALE    20/*1MB*/
    int64_t mem_used        = (mem_total - mem_free - mem_cache - mem_buffer);

    if (mask & SYSMEM_TOTAL) {
        sdt.total_mb        = (uint32_t)(mem_total >> SYSMEM_SCALE);
        stream(& sdt.total_mb, sizeof(sdt.total_mb));
    }
    if (mask & SYSMEM_USED) {
        sdt.used_mb         = (uint32_t)(mem_used >> SYSMEM_SCALE);
        stream(& sdt.used_mb, sizeof(sdt.used_mb));
    }
    if (mask & SYSMEM_USAGE) {
        // sdt.usage        = ;
        stream(& sdt.usage, sizeof(sdt.usage));
    }
    if (mask & SYSMEM_FREE) {
        sdt.free_mb         = (uint32_t)(mem_free >> SYSMEM_SCALE);
        stream(& sdt.free_mb, sizeof(sdt.free_mb));
    }
    if (mask & SYSMEM_SHARED) {
        sdt.shared_mb       = (uint32_t)(mem_shared >> SYSMEM_SCALE);
        stream(& sdt.shared_mb, sizeof(sdt.shared_mb));
    }
    if (mask & SYSMEM_CACHE) {
        sdt.cache_mb        = (uint32_t)(mem_cache >> SYSMEM_SCALE);
        stream(& sdt.cache_mb, sizeof(sdt.cache_mb));
    }
    if (mask & SYSMEM_SWAP_TOTAL) {
        sdt.swap_total_mb   = (uint32_t)(swap_total >> SYSMEM_SCALE);
        stream(& sdt.swap_total_mb, sizeof(sdt.swap_total_mb));
    }
    if (mask & SYSMEM_SWAP_USED) {
        sdt.swap_free_mb    = (uint32_t)(swap_free >> SYSMEM_SCALE);
        stream(& sdt.swap_free_mb, sizeof(sdt.swap_free_mb));
    }
    if (mask & SYSMEM_SWAP_USAGE) {
        // sdt.swap_usage   = ;
        stream(& sdt.swap_usage, sizeof(sdt.swap_usage));
    }
    if (mask & SYSMEM_HUGEPAGES_TOTAL) {
        // sdt.huage_page_total= ;
        stream(& sdt.huage_page_total, sizeof(sdt.huage_page_total));
    }
    if (mask & SYSMEM_HUGEPAGES_FREE) {
        // sdt.huage_page_free = ;
        stream(& sdt.huage_page_free, sizeof(sdt.huage_page_free));
    }
    if (mask & SYSMEM_HUGEPAGES_RSVD) {
        // sdt.huage_page_rsvd = ;
        stream(& sdt.huage_page_rsvd, sizeof(sdt.huage_page_rsvd));
    }
    if (mask & SYSMEM_HUGEPAGES_SURP) {
        // sdt.huage_page_surp = ;
        stream(& sdt.huage_page_surp, sizeof(sdt.huage_page_surp));
    }
    if (mask & SYSMEM_HUGEPAGES_USAGE) {
        // sdt.huage_page_usage= ;
        stream(& sdt.huage_page_usage, sizeof(sdt.huage_page_usage));
    }

    if (mask & SYSMEM_NUMAX){
        ;//pass
    }
    else{
        return;
    }

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

    for(;; i_node += 1){
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

        bool need0          = bitop_bmp_get(guide->mask_bmp_numa_total, node_id);
        bool need1          = bitop_bmp_get(guide->mask_bmp_numa_used , node_id);
        bool need2          = bitop_bmp_get(guide->mask_bmp_numa_usage, node_id);
        bool need3          = bitop_bmp_get(guide->mask_bmp_numa_free , node_id);
        bool needx          = need0 || need1 || need2 || need3;

        if (needx == false){
            continue;
        }

        sdt.i_numa          = node_id;
        sdt.mask_numa       = 
            (uint16_t)(need0) << I_SYSMEM_NUMAX_TOTAL |
            (uint16_t)(need1) << I_SYSMEM_NUMAX_USED  |
            (uint16_t)(need2) << I_SYSMEM_NUMAX_USAGE |
            (uint16_t)(need3) << I_SYSMEM_NUMAX_FREE;
        stream(& sdt.i_numa, sizeof(sdt.i_numa));
        stream(& sdt.mask_numa, sizeof(sdt.mask_numa));

        if (need0){
            sdt.numa_total  = mem_total >> SYSMEM_SCALE;
            stream(& sdt.numa_total, sizeof(sdt.numa_total));
        }
        if (need1){
            sdt.numa_used   = mem_used >> SYSMEM_SCALE;
            stream(& sdt.numa_used, sizeof(sdt.numa_used));
        }
        if (need2){
            sdt.numa_usage  = 1.0f * mem_used / mem_total;
            stream(& sdt.numa_usage, sizeof(sdt.numa_usage));
        }
        if (need3){
            sdt.numa_free   = mem_free >> SYSMEM_SCALE;
            stream(& sdt.numa_free, sizeof(sdt.numa_free));
        }
    }

    sdt.i_numa              = SYSDATA_END;
    stream(& sdt.i_numa, sizeof(sdt.i_numa));

    #undef  SYSMEM_UNIT_PH
    #undef  SYSMEM_PATH_NODE
    #undef  SYSMEM_SCALE
}

static void syswatch_tx_ioinfo_core(
    sysio_fetch_guide_item *    guide, 
    const sysio_stat *          stat, 
    syswatch_stream_invoke      stream){

    typedef sysio_data_template sdt_t;
    sdt_t          sdt                      = {};
    sysio_statex * last                     = & guide->stat;
    sysio_period * period                   = & guide->period;
    uint32_t       mask;

    if (guide->mask){
        mask                                = guide->mask;
        guide->mask                         = 0;
        stream(& guide->mask, sizeof(guide->mask));
    }
    else{
        return;
    }

    if (mask & SYSIO_RRQMX){
        sdt.rrqm                            = 
            (float)(stat->read_merged - last->read_merged_for_rrqm) / period->s_rrqm;
        last->read_merged_for_rrqm          = stat->read_merged;
        stream(& sdt.rrqm, sizeof(sdt.rrqm));
    }
    if (mask & SYSIO_WRQMX){
        sdt.wrqm                            = 
            (float)(stat->write_merged - last->write_merged_for_wrqm) / period->s_wrqm;
        last->write_merged_for_wrqm         = stat->write_merged;
        stream(& sdt.wrqm, sizeof(sdt.wrqm));
    }
    if (mask & SYSIO_RX){
        sdt.r                               = 
            (float)(stat->read_completed - last->read_completed_for_r) / period->s_r;
        last->read_completed_for_r          = stat->read_completed;
        stream(& sdt.r, sizeof(sdt.r));
    }
    if (mask & SYSIO_WX){
        sdt.w                               = 
            (float)(stat->write_completed - last->write_completed_for_w) / period->s_w;
        last->write_completed_for_w         = stat->write_completed;
        stream(& sdt.w, sizeof(sdt.w));
    }
    if (mask & SYSIO_RKBX){
        sdt.rkB                             = 
            (float)(stat->read_sectors - last->read_sectors_for_rkB) * SECTOR_BYTES / 1024/*KB*/ / period->s_rkB;
        last->read_sectors_for_rkB          = stat->read_sectors;
        stream(& sdt.rkB, sizeof(sdt.rkB));
    }
    if (mask & SYSIO_WKBX){
        sdt.wkB                             = 
            (float)(stat->write_sectors - last->write_sectors_for_wkB) * SECTOR_BYTES / 1024/*KB*/ / period->s_wkB;
        last->write_sectors_for_wkB = stat->write_sectors;
        stream(& sdt.wkB, sizeof(sdt.wkB));
    }
    if (mask & SYSIO_AVGRG_SZX){ // average request size
        sdt.avgrg_sz                        = 
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
        stream(& sdt.avgrg_sz, sizeof(sdt.avgrg_sz));
    }
    if (mask & SYSIO_AVGQU_SZX){
        // TODO: ==========================================================================================
        stream(& sdt.avgqu_sz, sizeof(sdt.avgqu_sz));
    }
    if (mask & SYSIO_SVCTMX){
        sdt.svctm                           =
            (float)(stat->io_ms - last->io_ms_for_svctm) / 
            (float)(stat->read_completed + stat->write_completed - 
                last->read_completed_for_scvtm - last->write_completed_for_scvtm);
        last->io_ms_for_svctm               = stat->io_ms;
        last->read_completed_for_scvtm      = stat->read_completed;
        last->write_completed_for_scvtm     = stat->write_completed;
        stream(& sdt.svctm, sizeof(sdt.svctm));
    }
    if (mask & SYSIO_UTILX){
        sdt.util                            =
            (float)(stat->io_ms - last->io_ms_for_util) / period->s_util;
        last->io_ms_for_util                = stat->io_ms;
        stream(& sdt.util, sizeof(sdt.util));
    }
    if (mask & SYSIO_AWAITX){
        sdt.await                           =
            (float)(stat->read_ms + stat->write_ms - last->read_ms_for_await - last->write_ms_for_await) / 
            (float)(stat->read_completed + stat->write_completed - 
                last->read_completed_for_await - last->write_completed_for_await);
        last->read_ms_for_await             = stat->read_ms;
        last->write_ms_for_await            = stat->write_ms;
        last->read_completed_for_await      = stat->read_completed;
        last->write_completed_for_await     = stat->write_completed;
        stream(& sdt.await, sizeof(sdt.await));
    }
}

extern void syswatch_get_ioinfo(sysio_fetch_guide * guide, syswatch_stream_invoke stream){
    if (guide->needed){
        guide->needed       = false;
    }
    else{
        return;
    }

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
    sysio_stat      stat    = {};
    int64_t       * io_stat = (int64_t *) & stat;
    sysio_fgi     * sfgi;
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

    for(; NULL != (sub = readdir(dev)); i++){
        if (bitop_bmp_get(guide->mask_bmp_disk, i) == false){
            continue;
        }
        else{
            bitop_bmp_reset(guide->mask_bmp_disk, i);
        }

        sprintf(path_sub, "%s/dev", sub->d_name);
        fd                  = fopen(path_dev, "r");

        // ERR
        if (fd == NULL){
            continue;
        }

        fscanf(fd, "%d:%d", & i_master, & i_slaver);
        fclose(fd);
        // TODO: make id

        sprintf(path_sub, "%s/stat", sub->d_name);
        fd                  = fopen(path_dev, "r");

        if (fd == NULL){
            // ERR
        }

        sfgi                = & guide->disk[i];
        i_stat              = (0);
        io_stat             = (int64_t *)& stat;

        while(i_stat < sizeof(stat) / sizeof(int64_t)){
            // ERR
            if (fscanf(fd, " %lld", & io_stat[i_stat]) <= 0){
                break;
            }

            i_stat         += 1;
        }

        fclose(fd);
        syswatch_tx_ioinfo_core(sfgi, & stat, stream);
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
