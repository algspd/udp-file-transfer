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
#include "udp.h"
#include "udpft.h"

int process_requests();


/**************************************************************************/
/* MAIN                                                                   */
/**************************************************************************/

int main(int argc,char **argv)
{
    int        s,port;

    port=atoi(argv[1]);
    
    /* start server up */
    s = start_server(port);
    if (s>=0){
      printf("Server started on port %i\n",port);
    }
    if (s==-1){
      printf("Port reserved or out-of-range\n");
      exit(1);
    }
    if (s==-2){
      printf("Error creating socket\n");
      exit(1);
    }

    process_requests(s);

    fprintf(stderr,"sftp server exiting!\n");
    return(1);
}

/**************************************************************************/
/* handle all requests -- does not return unless fatal error              */
/**************************************************************************/

int process_requests(int s){
  char buffer[10000];
  struct fgetinfo p1;
  struct fgetfrag p2;
  
  while (1) {
    if(receive(s,buffer,sizeof(buffer))==0){
      
      switch (packetType(buffer)){
         case 1:
            printf("Tipo 1\n");
            memcpy(&p1,buffer,sizeof(p1));
            printf("llega 1\n");
            if(check_fgetinfo(p1)){
               printf("Corrupted packet\n");
               exit (1);
            }
            else{
               printf("llega 2\n");
               print_fgetinfo(p1);
               printf("llega 3\n");
            }
            break;
         case 2:
            printf("Tipo 2\n");
            memcpy(&p2,buffer,sizeof(p2));
            if(check_fgetfrag(p2)){
               printf("Corrupted packet\n");
            }
            else{
               print_fgetfrag(p2);
            }
            break;
         default:
            printf("Unknown packet received\n");
      }

      
    }
    else{
      printf("Error receiving\n");
    }
  }
}



