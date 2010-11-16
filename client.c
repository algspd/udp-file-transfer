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
int transfer();

int main (int argc,char **argv){
   struct sockaddr_in server;
   struct fgetinfo    *p1;
   struct finfo       p4;
   char               buffer[1000];
   FILE *foutfd;
   
   char host[1000],fin[FILENAME_MAX],fout[FILENAME_MAX];
   int sock,port,connected,retval;
 
   parse(argc,argv,host,&port,fin,fout);
   
   
   /* PRINT PARSED ARGUMENTS */
   printf("Port %i<--\n",port);
   printf("Server %s<--\n",host);
   
   
   start_client(&sock,&server,port,host);

   printf("Starting communication\n\n");

   p1=get_info(fin);

   // Try to open out file
   if((foutfd=(FILE *)fopen(fout,"wb"))==NULL){
      printf("Out file could't be created or truncated for writing\n");
      exit(1);
   }
   
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
               // Start transfer
               retval=transfer(sock,&server,p4.file_id,foutfd);
               // Show transfer result
               exit(retval);
            }
            else{
               printf("Requested file does not exist on server\n");
               exit(1);
            }
         }
         connected++;
      }
   }
   if (connected==3){
      printf("Connection error\n");
      exit(1);
   }
   exit (0);
}


/* PARSE COMMAND LINE ARGUMENTS */
void parse(int argc,char **argv,char *host,int *port,char *fin,char *fout){
   char usage[40]="port host remote_file local_file\n\0";
   if (argc<5){
      fprintf(stderr,"Usage: %s %s",*argv,usage);
      exit (1);
   }
   *port=atoi(argv[1]);
   strcpy(host,argv[2]);
   strcpy(fin,argv[3]);
   strcpy(fout,argv[4]);
}


/* TRANSFER */
int transfer(int sock,struct sockaddr_in *server,int file_id,FILE *foutfd){
   struct fgetfrag *req;
   struct ffrag    ans;
   int end=0;
   while (!end){
      req=get_frag(file_id,42);
      send_buf(sock,server,req,sizeof(*req));
      // FIXME: change by select()
      printf("Now waiting for answer\n");
      if(receive(sock,(void *)&ans,sizeof(ans))==0){
         if(check_ffrag(ans)){
            // Answer correct
            printf("right!!!!\n");
         }
         else{
            printf("NOT right\n");
         }
      }
      // check received data
      // write to file
      end=1;
   }
   return 0;
}



