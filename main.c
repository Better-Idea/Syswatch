// #include"include/syswatch.h"

// syscpu_data cpu_data;
// sysmem_data mem_data;
// #include<stdbool.h>
// #include<sys/socket.h>
// #include<sys/types.h>
// #include<unistd.h>
// #include<netinet/in.h>
// #include<arpa/inet.h>
// #include<stdio.h>

// void server(){
//     int fd              = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//     int fd_user         = -1;
//     socklen_t   len     = 0;
//     struct
//     sockaddr_in user    = {};
//     struct
//     sockaddr_in sai     = {};
//     sai.sin_family      = AF_INET;
//     sai.sin_addr.s_addr = htonl(INADDR_ANY);
//     sai.sin_port        = htons(8848);
    
//     bind(fd, (struct sockaddr *)& sai, sizeof(sai));
//     listen(fd, 128);

//     while(true){
//         #define HW      "hello world!"
//         fd_user         = accept(fd, (struct sockaddr *)& user, & len);
//         write(fd_user, HW, sizeof(HW));
//         close(fd_user);
//         printf("%d:%d\n", user.sin_addr.s_addr, user.sin_port);
//     }
// }

// void client(){
//     char buf[100];
//     int  fd             = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//     int  fd_server      = -1;
//     struct
//     sockaddr_in sai     = {};

//     sai.sin_family      = AF_INET;
//     sai.sin_addr.s_addr = inet_addr("127.0.0.1");
//     sai.sin_port        = htons(8848);

//     while(true){
//         fd_server       = connect(fd, (struct sockaddr *) & sai, sizeof(sai));
//         read(fd_server, buf, sizeof(buf));
//         close(fd_server);
//         printf("%s\n", buf);
//         sleep(1);
//     }
// }


// void syswatch_waitms(uint32_t ms){
//     // ...
// }

#include"include/syswatch.h"

#include<stdio.h>
#include<string.h>


int main()
{
    sysnet_field field;
    // syswatch_get_netinfo_core("ens33", & field, 
    //     SYSNET_DUPLEX                   |
    //     SYSNET_AUTONEG                  |
    //     SYSNET_SPEED                    |
    //     SYSNET_STATUS                   |
    //     SYSNET_RXBYTES                  |
    //     SYSNET_RX_PACKETS               |
    //     SYSNET_RX_DROPPED               |
    //     SYSNET_RX_DROPPED_PERCENT       |
    //     SYSNET_RX_ERRS                  |
    //     SYSNET_RX_FIFO_ERRS             |
    //     SYSNET_RX_FRAME_ERRS            |
    //     SYSNET_RX_MCST                  |
    //     SYSNET_TX_BYTES                 |
    //     SYSNET_TX_PACKETS               |
    //     SYSNET_TX_DROPPED               |
    //     SYSNET_TX_DROPPED_PERCENT       |
    //     SYSNET_TX_ERRS                  |
    //     SYSNET_TX_FIFO_ERRS             |
    //     SYSNET_TX_COLL                  |
    //     SYSNET_TX_CARR
    // );

    return 0;
}

// int main(int argc, const char ** argv){
//     syswatch_init();
//     // syswatch_get_cpuinfo(& cpu_data, SYSCPU_SOCKET_NUM | SYSCPU_CORE_NUM | SYSCPU_LOAD | SYSCPU_ONLINE | SYSCPU_USAGE);
//     // syswatch_get_meminfo(& mem_data);
//     return 0;
// }