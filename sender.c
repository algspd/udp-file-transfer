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

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>

#include "udp.h"
#include "udpft.h"

#define MAXNAMELEN 256

int send_buf();

int main (int argc,char **argv){
   struct sockaddr_in server;
   struct hostent *host_info;
   struct fgetinfo *p;
   
   char SERVER[MAXNAMELEN],*server1;
   int sock,SERVER_PORT;
   char usage[100]="port@host\n\0";


   /* PARSE COMMAND LINE ARGUMENTS */
   if (argc<2){
      fprintf(stderr,"Usage: %s %s",*argv,usage);
       return 1;
   }
   /* Parse port@host */
   if((SERVER_PORT=atoi(strtok(argv[1],"@")))<=0){
      fprintf(stderr,"Port incorrect\n");
      exit(1);
   }

   server1=strtok(NULL,"@");
   strcpy(SERVER,server1);
   
   /* PRINT PARSED ARGUMENTS */
   printf("Port %i<--\n",SERVER_PORT);
   printf("Server %s<--\n",SERVER);
   
   
   /* CREATE SOCKET AND SET SERVER INFO */
   sock=socket(AF_INET, SOCK_DGRAM, 0);
   if (sock < 1) 
   {
      fprintf(stderr,"Error creating socket");
      return 1;
   }
   host_info=gethostbyname(SERVER);
   if (host_info==NULL) 
   {
      fprintf(stderr, "Unknown host: %s\n",SERVER);
      return 1;
   }
   server.sin_family=AF_INET;
   memcpy((char *)&server.sin_addr, host_info->h_addr, host_info->h_length);
   memset(&server.sin_zero, '\0', 8);
   server.sin_port=htons(SERVER_PORT);

   printf("Starting communication\n\n");

   p=get_info("example_path");
   
   printf("El path enviado es %s\n",p->file_path);
   printf("El md5 enviado es %s\n",p->md5);
   
   send_buf(sock,&server,p,sizeof(*p));
   
   exit (0);

}



