//
// Created by arvin on 2016/4/23.
//

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include "proto.h"
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
//#include "../cJSON.h"

unsigned char mac_id[32];
int count = 0;
void segfault_sigaction(int signal, siginfo_t *si, void *arg)
{
    printf("segfault\n");
    printf("or somebody want to kill me\n");
    exit(0);
}
DEFAULT_SETTING def_set;
void restore_default()
{
    //unsigne
    unsigned char tmp[1024];
    memset(tmp, 0 ,sizeof(tmp));
    //if(strlen(&(def_set.default_route)))
    snprintf(tmp, 1023, "route add default gw %s", &(def_set.default_route));
    system(tmp);
    if(def_set.is_vpn_initiated)
    {
        memset(tmp, 0, sizeof(tmp));
        snprintf(tmp, 1023, "iptables -t nat -I POSTROUTING -o %s -j MASQUERADE", &(def_set.vpn_if));
        system(tmp);
    }
}
int get_default_route2(int just_test)
{
    char devname[64], flags[16], sdest[16], sgw[16];
    unsigned long int d, g, m;
    int flgs, ref, use, metric, mtu, win, ir, flag = 0;
    struct in_addr mask, dest, gw;
    unsigned char *pos = NULL;
    FILE *fp = fopen("/proc/net/route", "r");
    if(fp == NULL)return -1;
    if (fscanf(fp, "%*[^\n]\n") < 0) { /* Skip the first line. */
        return -1;        /* Empty or missing line, or read error. */
    }
    flag = 0;
    while (1) {
        int r;
        r = fscanf(fp, "%63s%lx%lx%X%d%d%d%lx%d%d%d\n",
                   devname, &d, &g, &flgs, &ref, &use, &metric, &m,
                   &mtu, &win, &ir);
        if (r != 11) {
            if ((r < 0) && feof(fp)) { /* EOF with no (nonspace) chars read. */
                break;
            }
        }
        if (!(flgs & RTF_UP)) { /* Skip interfaces that are down. */
            continue;
        }
        dest.s_addr = d;
        memset(sdest, 0 , sizeof(sdest));
        memset(sgw, 0, sizeof(sgw));
        pos = NULL;
        if(just_test == 0)//only run once when startup
        {
            if(d == 0x00000000 && m == 0x00000000 && flag == 0)
            {
                gw.s_addr = g;
                strcpy(def_set.default_route, inet_ntoa(gw));
                flag = 1;//default got
                goto FINAL;
            }
            /*
            pos = strstr(devname, "ppp");
            if(pos != NULL)
            {
                def_set.is_vpn_initiated = 1;
                def_set.vpn_if[0] = 'p';def_set.vpn_if[1] = 'p';def_set.vpn_if[2] = 'p';def_set.vpn_if[3] = *(pos+3);
            }*/
        }
        else if(just_test == 1)
        {
            if(d == 0x00000000 && m == 0x00000000)
            {
                flag = 1;
                goto FINAL;
            }
        }
        else if(just_test == 2)
        {
            if(strstr(devname, "ppp") != NULL)
            {
                flag = 1;
                goto FINAL;
            }
        }
        //bb_printf("%-16s%-16s%-16s%-6s", sdest, sgw, inet_ntoa(mask), flags);
        //bb_printf("%-6d %-2d %7d %s\n", metric, ref, use, devname);
    }
    FINAL:
    fclose(fp);
    if(flag)return just_test;
    return -2;
}
//TODO: kill this func
int get_default_route(int just_test)
//0 get route
//1 test route
//2 test ppp
{
    return get_default_route2(just_test);
    unsigned char *line = NULL, *pos = NULL;
    FILE *fp = NULL; //= freopen("/tmp/my_route", "w");
    int read = 0, len = 0, i = 0, flag = 0, count = 0, ret = 0;
    system("route -n 1> /tmp/my_route");
    fp = fopen("/tmp/my_route", "r");
    if(fp == NULL)return -1;
    while((read = getline(&line, &len, fp)) != -1)
    {
        count++;
        if(just_test == 2)
        {
            pos = strstr(line, "ppp");
            if(pos != NULL)
            {
                ret = 2;
                goto final;
            }
        }
        else
        {
            pos = strstr(line, "default");
            i = 0;flag = 0;
            if(pos != NULL)
            {
                if(just_test == 1)
                {
                    ret = 1;
                    goto final;
                }
                pos += 7;
                while((*pos) != 0)
                {
                    if(((*pos)>='0' && (*pos)<='9') || (*pos) == '.')
                    {
                        flag = 1;
                        def_set.default_route[i++] = (*pos);
                        pos++;
                    }
                    else
                    {
                        if(flag == 1)break;
                        else pos++;
                    }
                }
                pos = strstr(line, "ppp");
                if(pos != NULL)
                {
                    //if(just_test == 2)return 2;
                    def_set.is_vpn_initiated = 1;
                    def_set.vpn_if[0] = 'p';def_set.vpn_if[1] = 'p';def_set.vpn_if[2] = 'p';def_set.vpn_if[3] = *(pos+3);
                }
                break;
            }
            pos = NULL;
            pos = strstr(line, "0.0.0.0");
            if(pos != NULL && pos <= line+4)
            {
                if(just_test == 1)
                {
                    ret = 1;
                    goto final;
                }
                pos += 8;
                i = 0;flag = 0;
                while((*pos) != 0)
                {
                    if(((*pos)>='0' && (*pos)<='9') || (*pos) == '.')
                    {
                        flag = 1;
                        def_set.default_route[i++] = (*pos);
                        pos++;
                    }
                    else
                    {
                        if(flag == 1)break;
                        else pos++;
                    }
                }
                pos = strstr(line, "ppp");
                if(pos != NULL)
                {
                    def_set.is_vpn_initiated = 1;
                    def_set.vpn_if[0] = 'p';def_set.vpn_if[1] = 'p';def_set.vpn_if[2] = 'p';def_set.vpn_if[3] = *(pos+3);
                }
                continue;
            }
        }
    }
    ret = 0;
    if(count <= 1 && just_test == 0)
    {
        ret = -1;
    }
    final:
    if(line != NULL)free(line);
    fclose(fp);
    //if(just_test == 0)printf("default route:%s ppp:%s\n", &(def_set.default_route), &(def_set.vpn_if));
    return ret;
}
int get_status(int sockfd)
{
    unsigned char *buf = NULL;
    int pfd[2];
    pipe(pfd);//what's the pipe's buffer size???
    if(fork() == 0)
    {
        //child

        close(pfd[0]);
        dup2(pfd[1], 0);
        close(pfd[1]);
        dup2(0, 1);
        dup2(0, 2);

        //close(pfd[0]);
        //close(1);
        //dup(pfd[1]);
        //close(pfd[1]);
        //printf("\n");
        fflush(stdout);
        system("ifconfig");
        printf("|||\n");
        fflush(stdout);
        system("route -n");
        printf("|||\n");
        fflush(stdout);
        system("ps");
        printf("|||\n");
        fflush(stdout);
        system("cat /proc/net/arp");
        printf("|||\n");
        fflush(stdout);
        printf("\n--Over--\n");
        exit(0);
    }
    else
    {
        //parent
        int len = 1;
        int total_len = 0;
        close(pfd[1]);
        buf = (signed char *)malloc(BUF_LEN * 2);
        int buf_size = BUF_LEN * 2;
        while(1)
        {
            len = read(pfd[0], buf+total_len, buf_size-total_len);
            if(len > 0)total_len += len;
            else break;
            if(total_len == buf_size)
            {
                buf = realloc(buf, buf_size+BUF_LEN);
                buf_size = buf_size+BUF_LEN;
            }
        }
        close(pfd[0]);
        if(i_send_data(sockfd, buf, total_len, RET_STATUS) < 0)
        {
            free(buf);
            return -1;
        }
        free(buf);
    }
    return 0;
}

void _system(unsigned char *cmd)
{
    unsigned char tmp[512];
    unsigned char *pos = NULL;
    memset(tmp, 0, sizeof(tmp));
    pos = strstr(cmd, "top");
    if(pos != NULL && pos <= cmd+10)
    {
        system("top -n 1");
        return;
    }
    pos = strstr(cmd, "ping");
    if(pos != NULL && pos <= cmd+13)
    {
        snprintf(tmp, 511, "ping -c 4 %s", cmd+5);
        system(tmp);
        return;
    }
    system(cmd);
    return;
}
int dispatch(int sockfd, I_PROTO_HEADER *header, unsigned char *data)
{
    //
    unsigned char tmp[100];
    switch(header->cmd)
    {
        case SOCK_TUNNEL:
        case RUN_CMD:
        {
            unsigned char *buf = NULL;

            int pfd[2];
            pipe(pfd);//what's the pipe's buffer size???
            if(fork() == 0)
            {
                //child
                close(pfd[0]);
                dup2(pfd[1], 0);
                close(pfd[1]);
                dup2(0, 1);
                dup2(0, 2);
                //execlp(data, data, NULL);//terminal,data data because it is linked to busybox need data again to tell busybox which command i need
                //FILE *wfd = fdopen(pfd[1], "w");
                printf("%s|||\n", data);
                fflush(stdout);
                //fclose(wfd);
                _system(data);
                printf("\n--Over--\n");
                exit(0);
            }
            else
            {
                //parent
                int len = 1;
                int total_len = 0;
                close(pfd[1]);
                buf = (signed char *)malloc(BUF_LEN * 2);
                int buf_size = BUF_LEN * 2;
                while(1)
                {
                    len = read(pfd[0], buf+total_len, buf_size-total_len);
                    if(len > 0)total_len += len;
                    else break;
                    if(total_len == buf_size)
                    {
                        buf = realloc(buf, buf_size+BUF_LEN);
                        buf_size = buf_size+BUF_LEN;
                    }
                }
                close(pfd[0]);
                if(i_send_data(sockfd, buf, total_len, RETURN_DATA) < 0)
                {
                    free(buf);
                    return -1;
                }
                free(buf);
            }
            //if(i_send_data(sockfd, test_buf, 3, RETURN_DATA) < 0)return -1;
            //test
            sleep(10);
            break;
        }
        case GET_BIN_WITH_SIZE:
        {
            //
            READ_BIN *param = (READ_BIN *)data;
            int size = 0, bin_ret = 0, ret = 0;
            //FILE *bin_fd = NULL;
            int bin_fd = 0;
            unsigned char *bin_buf = NULL;
            size = ntohl(param->size);
            bin_buf = (unsigned char *)malloc(size);
            //bin_fd = fopen(param->path, "rb");
            bin_fd = open(param->path, O_RDONLY);
            //if(bin_fd < 0)printf("err:%d\n", errno);
            ret = read(bin_fd, bin_buf, size);
            //printf("%d %d %s %d len:%d\n",ret , size, param->path, errno, strlen(param->path));
            //ret = fread(bin_buf, size, 1, bin_fd);
            if(ret < 0)break;
            if(i_send_data(sockfd, bin_buf, size, SAVE_TO_FILE) < 0)
            {
                close(bin_fd);
                free(bin_buf);
                return -1;
            }
            close(bin_fd);
            free(bin_buf);
            break;
        }
        case GET_STATUS:
        {
            //
            get_status(sockfd);
            sleep(10);
            break;
        }
        case CONNECT_VPN:
        {
            if(get_default_route(2) == 2)
            {
                //there already have a ppp0
                break;
            }
            memset(tmp, 0, sizeof(tmp));
            snprintf(tmp, 99, "route add -net %s netmask 255.255.255.255 gw %s", (def_set.server_ip), (def_set.default_route));
            system(tmp);
            system(data);
            if(fork() == 0)
            {
                int k = 0;
                while(get_default_route(2) != 2)
                {
                    k++;
                    sleep(1);
                    if(k > 90)break;
                }
                system("iptables -t nat -I POSTROUTING -j MASQUERADE");
                system("iptables -I FORWARD -j ACCEPT");
                if(fork() == 0)
                {
                    //iptables daemon
                }
                exit(0);
            }
            else
            {
                system("iptables -t nat -I POSTROUTING -j MASQUERADE");
                system("iptables -I FORWARD -j ACCEPT");
            }
            break;
        }
        case DISCONNECT_VPN:
        {
            restore_default();
            memset(tmp, 0, sizeof(tmp));
            snprintf(tmp, 99, "kill %s", data);
            system(tmp);
            break;
        }
        case CHANGE_DEFAULT:
        {
            memset(tmp, 0, sizeof(tmp));
            snprintf(tmp, 99, "route add default gw %s", data);
            system("route del default");
            system(tmp);
            break;
        }
        case RESTORE_DEFAULT:
        {
            restore_default();
        }
        case IDLE:
        default:
            sleep(SLEEP_TIME);
    }
    return 0;
}

int i_client(int sockfd)
{
    int len = 0, data_len = 0, cmd = 0;
    unsigned char *data_buf = NULL;
    unsigned char buf[BUF_LEN] = {0};
    count = 1;
    while(1)
    {
        //
        if(mac_id[0] != 'R')
            get_mac(mac_id, 33, "eth0");
        len = 0;
        data_len = 0;
        data_buf = NULL;
        memset(buf, 0, sizeof(buf));
        //printf("send my header\n");
        i_send_data(sockfd, NULL, 0, GET_CMD);
        //printf("waiting for header\n");
        len = my_read(sockfd, buf, BUF_LEN, 60);
        I_PROTO_HEADER *header = (I_PROTO_HEADER *)buf;
        if(len < BUF_LEN || strncmp((header->token), TOKEN, strlen(TOKEN)) != 0)
            break;
        header->cmd = ntohl(header->cmd);
        header->length = ntohl(header->length);
        data_len = (header->length);
        //printf("cmd:%d,%s,len:%d\n", header->cmd, header->token, data_len);
        if(data_len != 0)
        {
            data_buf = (unsigned char *)malloc(data_len+1);//+1 is the \0
            memset(data_buf, 0, data_len+1);
            len = i_recv_data(sockfd, data_buf, data_len);
            if(len <= 0)
                break;
        }
        else
            data_buf = NULL;

        if(dispatch(sockfd, header, data_buf) < 0)
            break;
        if(data_buf != NULL)
        {
            free(data_buf);
            data_buf = NULL;
        }
        //close(sockfd);
    }
    if(data_buf != NULL)
    {
        free(data_buf);
        data_buf = NULL;
    }
    return 0;
}
char *get_confpath()
{
    //remember it is user #define
    if(__MODEL__ == TL_WR703N || __MODEL__ == TL_WR941N|| __MODEL__ == TL_WR720N)
    {
        return "/dev/mtdblock3";
    }
    else
    {
        return "";
    }
}
//2015-5-29
void *self_monitor(unsigned int pid)
{
    //
    char re[256] = {0};
    while(1)
    {
        //发送无效信号
        if(kill(pid, 0) == -1)
        {
            sprintf(re, "%s %s %s &", def_set.argv0, def_set.argv1, def_set.argv2);
            system(re);
            exit(0);
        }
        else
        {
            sleep(60);
        }
    }
}
unsigned char *get_config_ip()
{
    unsigned char buf[24] = {0};
    char *config_path = NULL;
    int sockfd,new_fd, len = 0, bin_fd;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    int sin_size,portnumber;
    sockfd=socket(AF_INET,SOCK_STREAM,0);
    bzero(&server_addr,sizeof(struct sockaddr_in));
    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    server_addr.sin_port=htons(7755);
    bind(sockfd,(struct sockaddr *)(&server_addr),sizeof(struct sockaddr));
    listen(sockfd,5);
    while(1)
    {
        sin_size=sizeof(struct sockaddr_in);
        if((new_fd=accept(sockfd,(struct sockaddr *)(&client_addr),&sin_size))==-1)
        {
            fprintf(stderr,"Accept error:%s\n\a",strerror(errno));
            continue;
        }
        len = my_read(new_fd, buf, 24, 180);
        if(len <= 0 )
        {
            close(new_fd);
            continue;
        }
        buf[23] = 0;
        if(buf[0] == 'w' && buf[1] == 'U')
        {
            //ok
            bin_fd = open(get_confpath(), O_RDWR);
            lseek(bin_fd,-24,SEEK_END);
            sync();
            write(bin_fd, buf, 24);
            close(bin_fd);
            close(new_fd);
            break;
        }
        else
        {
            close(new_fd);
            continue;
        }
    }
    return NULL;
}
int init_start(char *pwd)
{
    //bin_fd = open(param->path, O_RDONLY);
    unsigned char buf[32] = {0};
    char sys_start[128] = {0};
    char *config_path = get_confpath();
    int bin_fd = 0;
    bin_fd = open(get_confpath(), O_RDWR);
    lseek(bin_fd,-24,SEEK_END);
    read(bin_fd, buf, 24);
    if(buf[0] == 'w' && buf[1] == 'U')
    {
        sprintf(sys_start, "%s %s", pwd, buf+2);
        system(sys_start);
        exit(0);
    }
    else
    {
        //get config and save
        sprintf(sys_start, "%s", pwd);
        get_config_ip();
        system(sys_start);
        exit(0);
    }
    return 0;
}
int main(int argc, char *argv[])
{
    struct sigaction sa,sw;
    memset(&sa, 0, sizeof(sa));
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = segfault_sigaction;
    sa.sa_flags = SA_SIGINFO;
    sigaction(SIGSEGV, &sa, NULL);
    memset(&sw, 0, sizeof(sw));
    sw.sa_handler = SIG_IGN;
    sw.sa_flags = SA_NOCLDWAIT;
    sigemptyset(&sw.sa_mask);
    sigaction(SIGCHLD, &sw, NULL);
    memset(mac_id, 0, sizeof(mac_id));
    sleep(30);
    while(mac_id[0] != 'R')
    {
        sleep(1);
        get_mac(mac_id, 33, "eth0");
    }
    //sleep(10);
    int sockfd;
    char buffer[1024];
    struct sockaddr_in server_addr;
    struct hostent *host;
    int portnumber,nbytes;

    if(argc!=3)
    {
        //fprintf(stderr,"Usage:%s hostname portnumber\a\n",argv[0]);
        //init_start(argv[0]); //on 2015-3-16 no need
        exit(1);

    }

    while((host=gethostbyname(argv[1]))==NULL)
    {
        //fprintf(stderr,"Gethostname error\n");
        sleep(10);
    }

    if((portnumber=atoi(argv[2]))<0)
    {
        //fprintf(stderr,"Usage:%s hostname portnumber\a\n",argv[0]);
        exit(1);
    }
    memset(&def_set, 0, sizeof(def_set));
    def_set.argc = argc;
    strncpy(def_set.argv0, argv[0], 127);
    strncpy(def_set.argv1, argv[1], 63);
    strncpy(def_set.argv2, argv[2], 7);
    //def_set.argv[0] = argv[0];
    //def_set.argv[1] = argv[1];
    //def_set.argv[2] = argv[2];
    //fork monitor
    int self_pid = getpid();
    int child_pid = 0, try_num = 4;
    do
    {
        child_pid = fork();
        if(child_pid == 0)
        {
            self_monitor(self_pid);
            exit(0);
        }
        else if(child_pid > 0)
        {
            break;
        }
        else
        {
            if(try_num == 1)
            {
                system("reboot");
            }
            else
            {
                sleep(20);
            }
        }
    }
    while(try_num--);
    strncpy((def_set.server_ip), inet_ntoa(*(struct in_addr *)host->h_addr), 31);
    while(get_default_route(0) < 0)sleep(3);
    //init shadowsocks iptables rules
    //system("ipt-rule");//on 2015-3-16 no need
    while(1)
    {//have bus while server closed
        if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1)
        {
            //fprintf(stderr,"Socket Error:%s\a\n",strerror(errno));
            //exit(1);
        }
        bzero(&server_addr,sizeof(server_addr));
        server_addr.sin_family=AF_INET;
        server_addr.sin_port=htons(portnumber);
        server_addr.sin_addr=*((struct in_addr *)host->h_addr);
        while(connect(sockfd,(struct sockaddr *)(&server_addr),sizeof(struct sockaddr))==-1)
        {
            //printf("Connect Error:%s\a\n",strerror(errno));
            sleep(30);
        }
        //get_status(sockfd);//get status first;
        i_client(sockfd);
        close(sockfd);
        if(get_default_route(1) != 1)
        {
            restore_default();
        }
        sleep(30);
    }
    exit(0);
}