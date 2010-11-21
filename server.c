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
#include <unistd.h>

#include "udp.h"
#include "udpft.h"

int process_requests();


/**************************************************************************/
/* MAIN                                                                   */
/**************************************************************************/

int main(int argc,char **argv)
{
    int        s,port;

    if (argc<2){
      printf("Usage: %s port\n",argv[0]);
      exit(1);
    }
    
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
   struct sockaddr remote;
   int rlen,retval;
   int last_file_used=0,N;
   struct file_s file[100];
   
   struct fgetinfo p1;
   struct fgetfrag p2;
   struct finfo    *p4;
   struct ffrag    *p5;
  
   while (1) {
      rlen = sizeof(remote);
      if((recvfrom(s,buffer,sizeof(buffer),0,(struct sockaddr *)&remote,(socklen_t *)&rlen))>0){
         switch (packetType(buffer)){
            case 1:
               memcpy(&p1,buffer,sizeof(p1));
               if(check_fgetinfo(p1)){
                  printf("Corrupted packet\n");
               }
               else{
                  print_fgetinfo(p1);
                  printf("Now sending answer\n");
                  //check if file exists and can be open
                  printf("El fichero es: %s\n",p1.file_path);
                  N=last_file_used;
                  if((file[N].fd=(FILE *)fopen(p1.file_path,"r"))==NULL){
                     // If file not found tell the client
                     p4=get_sinfo(0,0,0);
                  }
                  else{
                     strncpy(file[N].file_path,p1.file_path,FILENAME_MAX);
                     file[N].file_id=N;
                     // If file found, tell the client info about it
                     fseek(file[N].fd, 0, SEEK_END);      // seek to end of file
                     file[N].size=ftell(file[N].fd); // get file size
                     // Has no sense to seek back to beginning of file
                     p4=get_sinfo(1,file[N].file_id,file[N].size);
                     N++;
                  }
                  // Send info about the file
                  if(reply(s,&remote,rlen,p4,sizeof(*p4))){
                     printf("Error sending\n");
                  }
               }
               break;
            case 2:
               memcpy(&p2,buffer,sizeof(p2));
               if(check_fgetfrag(p2)){
                  printf("Corrupted packet\n");
               }
               else{
                  print_fgetfrag(p2);
                  // Send fragment
                  fseek(file[p2.file_id].fd,SEEK_SET,p2.offset);
//                   memset(buffer,0,sizeof(buffer));
                  if((retval=fread(buffer,sizeof(char),sizeof(fra_t),file[p2.file_id].fd))<=0){
                     printf("Nothing read\n");
                     exit(1);
                  }
                  printf("Retval es %i\n",retval);
                  p5=get_ffrag(p2.file_id,p2.offset,buffer,retval);
                  if(reply(s,&remote,rlen,p5,sizeof(*p5))){
                     printf("Error sending\n");
                  }
               }
               break;
            default:
               // If first character of packet is unknown
               printf("Unknown/corrupt packet received\n");
         }   
      }
      else{
         printf("Error receiving\n");
      }
   }
}



