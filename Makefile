#   This file is part of "UDP file transfer".
#   
#   "UDP file transfer" is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   "UDP file transfer" is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with "UDP file transfer".  If not, see <http://www.gnu.org/licenses/>.
#

#
# Makefile for "UDP file transfer".
#
# Compiler: gcc 4.4.5
# Linker: GNU ld 2.20.1-system.20100303
# 

CC=gcc
CFLAGS=-Wall -lssl

all: server client
server: udp.o udpft.o md5.o
	$(CC) -o server server.c udp.o udpft.o md5.o $(CFLAGS)
client: udp.o udpft.o md5.o
	$(CC) -o client client.c udp.o udpft.o md5.o $(CFLAGS)
udpft.o: 
	$(CC) -c -g udpft.c $(CFLAGS)
udp.o:
	$(CC) -c -g udp.c $(CFLAGS)
md5.o:
	$(CC) -c -lssl -g md5.c $(CFLAGS)
clean:
	rm -f *.o *~
mrproper: clean
	rm -f `file *|grep ELF|awk '{print $1}'|sed 's/://'`
edit:
	/usr/bin/kate *.c *.h README Makefile > /dev/null 2>&1 &
