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

#include <openssl/evp.h>


#define ERRORSTRING "unknown error"
#define MAXNAMELEN 256

int send_buf();

char hexToAscii(char in)
{
  int cero='0';
  int laa='a';
  char foo;
  if (in<10){
    return(in+cero);
  }
  return(in+laa);
}

int main (int argc,char **argv){
   struct sockaddr_in server;
   struct hostent *host_info;
   char SERVER[MAXNAMELEN],*server1;
   int sock,SERVER_PORT,i;
   char usage[100]="port@host\n\0";
   char buf[10];
   EVP_MD_CTX mdctx;
   
   unsigned char foo[100];
   char *md;
   long n=1;
   strcpy(foo,"h");

      printf("long vale %li\n",n);
      printf("foo vale %s\n\n",foo);
      
      
//   static void calculate_md5_of(const void *content, ssize_t len){

  unsigned char md_value[EVP_MAX_MD_SIZE];
  unsigned int md_len;
  
  EVP_DigestInit(&mdctx, EVP_md5());
  EVP_DigestUpdate(&mdctx, foo, (size_t) 1);
  EVP_DigestFinal_ex(&mdctx, md_value, &md_len);
  EVP_MD_CTX_cleanup(&mdctx);


//    md=MD5(foo,n,NULL);

  for(i = 0; i < md_len; i++){
    printf ("%02x", md_value[i]);
  }

   
//    printf("md vale %d %d %d \n\n",md[0],md[1],md[2],md[3]);
   exit (2);

   /* PARSE COMMAND LINE ARGUMENTS */
   if (argc<2){
      fprintf(stderr,"Usage: %s %s",*argv,usage);
       return 1;
   }
   printf("argv1 es %s\n",argv[1]);   
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

   strcpy(buf,"probando\0");
  printf("El tamaño de buf es %i\n",sizeof(buf));
   send_buf(sock,&server,buf,sizeof(buf));

   exit (0);

}



