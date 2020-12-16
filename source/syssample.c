#include<arpa/inet.h>
#include<netinet/in.h>
#include<semaphore.h>
#include<stdatomic.h>
#include<stdbool.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<unistd.h>

#include"include/syswatch.h"

// extern "C"
#include"cbegin.h"
extern sys_scb *    list_sbc_ptr[];
extern const size_t list_sbc_num;
static sem_t        watcher_sem;
static int          watcher_list[SYSW_MAX_CONN];
static int          watcher_push_i;
static int          watcher_pop_i;

static void syswatch_heap_push(sys_scb ** list, size_t length, sys_scb * value){
    size_t    i         = length;
    size_t    ii;
    sys_scb * parent;

    while(i > 0){
        parent          = list[
            ii          = (i - 1) >> 1
        ];

        if (parent->wakeup_time < value->wakeup_time){
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

        if (left->wakeup_time > right->wakeup_time){
            select      = right;
            i_left     += 1;
        }
        else{
            select      = left;
        }

        if (select->wakeup_time >= last->wakeup_time){
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

void syswatch_server_exchange(){
    uint64_t  current;
    sys_scb * root;

    while(true){
        root            = list_sbc_ptr[0];
        current         = root->wakeup_time;
        sleep(root->time_interval);

        do{
            root        = syswatch_heap_pop(list_sbc_ptr, list_sbc_num, 0);
            root->wakeup_time
                       += root->time_interval;
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
