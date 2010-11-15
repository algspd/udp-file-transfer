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

#include <string.h>

#include "udp.h"
#include "udpft.h"

#define MAXNAMELEN 256

// int send_buf();
void parse();

int main (int argc,char **argv){
   struct sockaddr_in server;
   struct fgetinfo    *p1;
   struct finfo       p4;
   char               buffer[1000];
   
   char host[1000];
   int sock,port,connected;
 
   parse(argc,argv,host,&port);
   
   
   /* PRINT PARSED ARGUMENTS */
   printf("Port %i<--\n",port);
   printf("Server %s<--\n",host);
   
   
   start_client(&sock,&server,port,host);

   printf("Starting communication\n\n");

   p1=get_info("sender");
   connected=0;
   while (connected<3){
      send_buf(sock,&server,p1,sizeof(*p1));
      print_fgetinfo(*p1);
      
      if(receive(sock,buffer,sizeof(buffer))==0){
         memcpy(&p4,buffer,sizeof(p4));
         if(!check_finfo(p4)){
            connected=4;
            print_finfo(p4);
            if (p4.file_exist==1){
               //file is open with p4.file_id
               //add to proper struct and start transfer
               //create target file
               //start transfer
               //exit
            }
         }
         connected++;
      }
   }
   if (connected==3){
      printf("Connection error\n");
   }
   
//    p2=get_frag(7,42);
//    send_buf(sock,&server,p2,sizeof(*p2));
   
   exit (0);

}


/* PARSE COMMAND LINE ARGUMENTS */
void parse(int argc,char **argv,char *host,int *port){
   char usage[15]="port host\n\0";

   if (argc<3){
      fprintf(stderr,"Usage: %s %s",*argv,usage);
      exit (1);
   }

   *port=atoi(argv[1]);
   strcpy(host,argv[2]);
}



