#!/bin/sh
gcc -o sender sender.c udp.c
gcc -o sftpd sftpd2.c udp.c
