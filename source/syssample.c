#include<arpa/inet.h>
#include<netinet/in.h>
#include<semaphore.h>
#include<stdatomic.h>
#include<stdbool.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<unistd.h>

#include"include/syswatch.h"
#include"include/utils/bitop.h"

// extern "C"
#include"cbegin.h"
extern sys_scb *    list_sbc_ptr[];
extern const size_t list_sbc_num;
static sem_t        watcher_sem;
static int          watcher_list[SYSW_MAX_CONN];
static int          watcher_push_i;
static int          watcher_pop_i;

static int syswatch_cmp_sbc(const void * left, const void * right){
    sys_scb ** l        = (sys_scb **)left;
    sys_scb ** r        = (sys_scb **)right;

    if (l[0]->wakeup_time != r[0]->wakeup_time){
        return l[0]->wakeup_time > r[0]->wakeup_time ? 1 : -1;
    }
    if (l[0]->i_group != r[0]->i_group){
        return l[0]->i_group > r[0]->i_group ? 1 : -1;
    }
    if (l[0]->i_mask != r[0]->i_mask){
        return l[0]->i_mask > r[0]->i_mask ? 1 : -1;
    }
    if (l[0]->i_addition != r[0]->i_addition){
        return l[0]->i_addition > r[0]->i_addition ? 1 : -1;
    }

    // equals
    return 0;
}

static void syswatch_heap_push(sys_scb ** list, size_t length, sys_scb * value){
    size_t    i         = length;
    size_t    ii;
    sys_scb * parent;

    while(i > 0){
        parent          = list[
            ii          = (i - 1) >> 1
        ];

        if (syswatch_cmp_sbc(& parent, & value) < 0){
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


static sys_scb * syswatch_heap_pop(sys_scb ** list, size_t length, size_t index){
    sys_scb * left;
    sys_scb * right;
    sys_scb * select;
    sys_scb * last      = (list[length - 1]);
    sys_scb * wanted    = (list[index]);
    size_t    i         = (index);
    size_t    i_left    = (index << 1) + 1;

    while(i_left + 1 < length) {
        left            = list[i_left];
        right           = list[i_left + 1];

        if (syswatch_cmp_sbc(left, right) > 0){
            select      = right;
            i_left     += 1;
        }
        else{
            select      = left;
        }

        if (syswatch_cmp_sbc(select, last) >= 0){
            break;
        }

        list[i]         = (select);
        select->i_in_heap
                        = (i);
        i               = (i_left);
        i_left          = (i_left << 1) + 1;
    }

    list[i]             = (last);
    last->i_in_heap     = (i);
    return wanted;
}

void syswatch_watcher_init(){
    sem_init(& watcher_sem, 0/*non-shared*/, SYSW_MAX_CONN);
}

// note:just allow one producer thread use syswatch_watcher_push
void syswatch_watcher_push(int fd){
    int i;
    sem_wait(& watcher_sem);

    if (watcher_push_i >= SYSW_MAX_CONN){
        watcher_push_i  = 0;
    }

    i                   = watcher_push_i;
    watcher_push_i     += 1;
    atomic_store(& watcher_list[i], fd);
}

// note:just allow one consumer thread use syswatch_watcher_pop
int syswatch_watcher_pop(){
    int val;
    int i;
    sem_getvalue(& watcher_sem, & val);

    if (val == SYSW_MAX_CONN){
        return -1;
    }
    if (watcher_pop_i >= SYSW_MAX_CONN){
        watcher_pop_i   = 0;
    }

    i                   = watcher_pop_i;
    watcher_pop_i      += 1;
    val                 = atomic_load(& watcher_list[i]);
    sem_post(& watcher_sem);
    return val;
}

static syscpu_fetch_guide   guide_cpu;
static sysmem_fetch_guide   guide_mem;
static sysio_fetch_guide    guide_io;
static sysnet_fetch_guide   guide_net;

static void syswatch_merge_request_cpu(const sys_scb * item){
    guide_cpu.mask     |=  1 << item->i_mask;

    // non-array data
    if (item->i_mask < B_SYSCPU_XDATA){
        return;
    }

    size_t   i          = item->i_mask - B_SYSCPU_XDATA;
    size_t * table[]    = {
        [I_SYSCPU_ONLINEX - B_SYSCPU_XDATA] = guide_cpu.mask_bmp_cpu_online,
        [I_SYSCPU_USAGEX  - B_SYSCPU_XDATA] = guide_cpu.mask_bmp_cpu_usage,
    };

    bitop_bmp_set(table[i], item->i_addition);
}

static void syswatch_merge_request_mem(const sys_scb * item){
    guide_mem.mask     |= 1 << item->i_mask;

    // non-array data
    if (item->i_mask < B_SYSMEM_NUMA){
        return;
    }

    size_t      i       = item->i_mask - B_SYSMEM_NUMA;
    size_t *    table[] = {
        [I_SYSMEM_NUMA_TOTALX - B_SYSMEM_NUMA] = guide_mem.mask_bmp_numa_total,
        [I_SYSMEM_NUMA_USEDX  - B_SYSMEM_NUMA] = guide_mem.mask_bmp_numa_used ,
        [I_SYSMEM_NUMA_USAGEX - B_SYSMEM_NUMA] = guide_mem.mask_bmp_numa_usage,
        [I_SYSMEM_NUMA_FREEX  - B_SYSMEM_NUMA] = guide_mem.mask_bmp_numa_free ,
    };

    bitop_bmp_set(table[item->i_addition - B_SYSMEM_NUMA], item->i_addition);
}

static void syswatch_merge_request_io(const sys_scb * item){
    sysio_fgi * disk    = & guide_io.disk[item->i_mask];
    guide_io.needed     = true;
    bitop_bmp_set(guide_io.mask_bmp_disk, item->i_mask);
    disk->mask         |= 1 << item->i_addition;
}

static void syswatch_merge_request_net(const sys_scb * item){
    sysnet_fgi * eth    = & guide_net.eth[item->i_mask];
    guide_net.needed    = true;
    bitop_bmp_set(guide_net.mask_bmp_eth, item->i_mask);
    eth->mask          |= 1 << item->i_addition;
}

static void syswatch_merge_request(sys_scb * item){
    typedef void (* merge)(const sys_scb *);

    merge table[]  = {
        [I_SYSCPU] = & syswatch_merge_request_cpu,
        [I_SYSMEM] = & syswatch_merge_request_mem,
        [I_SYSIO ] = & syswatch_merge_request_io,
        [I_SYSNET] = & syswatch_merge_request_net,
    };

    table[item->i_group](item);
}

void syswatch_server_exchange(){
    uint64_t  current;
    sys_scb * root;

    while(true){
        root            = list_sbc_ptr[0];
        current         = root->wakeup_time;

        // TODO:replace by ms sleep
        // sleepms(root->ms_period);

        do{
            root        = syswatch_heap_pop(list_sbc_ptr, list_sbc_num, 0);
            root->wakeup_time
                       += root->ms_period;
            syswatch_merge_request(root);
            syswatch_heap_push(list_sbc_ptr, list_sbc_num - 1, root/*push*/);
        }while(list_sbc_ptr[0]->wakeup_time == current);

        
    }
}

void syswatch_server(){
    int fd              = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    int fd_user         = -1;
    socklen_t   len     = 0;
    struct
    sockaddr_in user    = {};
    struct
    sockaddr_in sai     = {};
    sai.sin_family      = AF_INET;
    sai.sin_addr.s_addr = htonl(INADDR_ANY);
    sai.sin_port        = htons(SYSW_PORT);

    if (fd <= 0){
        // ERR:
    }

    bind(fd, (struct sockaddr *)& sai, sizeof(sai));

    // SYSW_MAX_CONN just a suggestion for 'listen', the max connect number may not equals to this value
    listen(fd, SYSW_MAX_CONN);

    while(true){
        fd_user         = accept(fd, (struct sockaddr *)& user, & len);

        if (fd_user <= 0){
            continue;
        }

        syswatch_watcher_push(fd_user);
    }
}

void client(){
    char buf[100];
    int  fd             = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    int  fd_server      = -1;
    struct
    sockaddr_in sai     = {};

    sai.sin_family      = AF_INET;
    sai.sin_addr.s_addr = inet_addr("127.0.0.1");
    sai.sin_port        = htons(SYSW_PORT);

    while(true){
        fd_server       = connect(fd, (struct sockaddr *) & sai, sizeof(sai));
        read(fd_server, buf, sizeof(buf));
    }
}

#include"cend.h"
