#! /usr/bin/python
# -*- coding: utf-8 -*-
# @Author: arvin
# @Date:   2016-01-27 16:41:39
# @Last Modified by:   arvin
# @Last Modified time: 2016-01-27 21:54:45

import logging
logging.getLogger('scapy.runtime').setLevel(logging.ERROR)
from scapy.all import *

def full_scan(dst_ip, dst_port):
    src_port = RandShort()

    tcp_connect_scan_resp =  sr1(IP(dst=dst_ip)/TCP(sport=src_port, dport=dst_port, flags='S'), timeout=10)
    if tcp_connect_scan_resp is None:
        print 'closed'
    elif tcp_connect_scan_resp.haslayer(TCP):
        if tcp_connect_scan_resp.getlayer(TCP).flags == 0x12:
            # return AS
            send_rst = sr(IP(dst=dst_ip)/TCP(sport=src_port, dport=dst_port, flags='AR'), timeout=10)
            print 'open'
    elif tcp_connect_scan_resp.getlayer(TCP).flags == 0x14:
        print 'closed'

# error type: 3 Destination Unreachable
# error code:
# 0       Destination network unreachable
# 1       Destination host unreachable
# 2       Destination protocol unreachable
# 3       Destination port unreachable
# 4       Fragmentation required, and DF flag set
# 5       Source route failed
# 6       Destination network unknown
# 7       Destination host unknown
# 8       Source host isolated
# 9       Network administratively prohibited
# 10      Host administratively prohibited
# 11      Network unreachable for TOS
# 12      Host unreachable for TOS
# 13      Communication administratively prohibited
# 14      Host Precedence Violation
# 15      Precedence cutoff in effect

def syn_scan(dst_ip, dst_port):
    src_port = RandShort()

    tcp_connect_syn_scan_resp = sr1(IP(dst_ip=dst_ip)/TCP(sport=src_port, dport=dst_port, flags='S'), timeout=10)
    if tcp_connect_syn_scan_resp is None:
        print 'filtered'
    elif tcp_connect_syn_scan_resp.haslayer(TCP):
        if tcp_connect_syn_scan_resp(TCP).flags == 0x12:
            send_rst = sr(IP(dst_ip=dst_ip)/TCP(sport=src_port, dport=dst_port, flags='R'), timeout=10):
            print 'open'
        elif tcp_connect_syn_scan_resp.getlayer(TCP).flags == 0x14:
            print 'closed'

def fin_scan(dst_ip, dst_port):
    src_port = RandShort()

    fin_scan_resp = sr1(IP(dst_ip=dst_ip)/TCP(sport=src_port, dport=dst_port, flags='F'), timeout=10)
    if fin_scan_resp is None:
        print 'open|filtered'
    elif fin_scan_resp.haslayer(TCP):
        if fin_scan_resp.getlayer(TCP).flags == 0x14:
            print 'close'
    elif fin_scan_resp.haslayer(ICMP):
        if int(fin_scan_resp.getlayer(ICMP).type) == 3 and int(fin_scan_resp.getlayer(ICMP).code) in [1, 2, 3, 9, 10, 13]:
            print 'filtered'

def udp_sacn(dst_ip, dst_port):
    src_port = RandShort()
    udp_scan_resp = sr1(IP(dst_ip=dst_ip)/UDP(sport=src_port, dport=dst_port), timeout=10)
    if udp_scan_resp is None:
        # receive nothing, means dest port status is uncertain
        retrans = []
        for x in xrange(0, 3):
            # retry 3 times to make sure
            retrans.append(sr1(IP(dst_ip=dst_ip)/UDP(sport=src_port, dport=dst_port), timeout=10))
        for item in retrans:
            if item is not None:
                udp_scan_resp(dst_ip, dst_port)
        print 'open|filtered'
    elif udp_scan_resp.haslayer(UDP):
        # receive udp back
        print 'open'
    elif udp_scan_resp.haslayer(ICMP):
        if int(udp_scan_resp.getlayer(ICMP).type) == 3 and int(udp_scan_resp.getlayer(ICMP).code) == 3:
            print 'closed'
        elif int(udp_scan_resp.getlayer(ICMP).type) == 3 and int(udp_scan_resp.getlayer(ICMP).code) in [1, 2, 9, 10, 13]:
            return 'filtered'

if __name__ == '__main__':
    full_scan('www.baidu.com', 80)