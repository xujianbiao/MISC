#! /usr/bin/python
# -*- coding: utf-8 -*-
# @Author: arvin
# @Date:   2016-01-27 16:41:39
# @Last Modified by:   arvin
# @Last Modified time: 2016-01-27 17:13:37

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

if __name__ == '__main__':
    full_scan('www.baidu.com', 80)