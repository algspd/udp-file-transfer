/*
   This file is part of "UDP file transfer".
   
   "UDP file transfer" is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   "UDP file transfer" is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with "UDP file transfer".  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef UDP_H
   #define UDP_H
   #include <errno.h>
   #include <string.h>
   #include <arpa/inet.h>
#endif
#ifndef PORT_LAST
   #define PORT_LAST 65535
#endif

extern int send_buf(int sock,struct sockaddr_in *server,char *buf,int buflen);
extern int start_server(int port);
extern int receive(int s,char *rbuffer,int rbufferlen);