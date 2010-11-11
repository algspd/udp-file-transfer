#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/uio.h>
#include <time.h>
#include <math.h>

#define ERRORSTRING "unknown error"
#define MAXNAMELEN 256

int send_buf();

int main (int argc,char **argv){
   struct sockaddr_in server;
   struct hostent *host_info;
   char SERVER[MAXNAMELEN],*port,*server1;
   int sock,SERVER_PORT;
   char usage[100]="port@host\n\0";
   char buf[1000];

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

   strcpy(buf,"probandooaeiouaeiouooooouuuuuu\0");
  printf("El tamaño de buf es %i\n",sizeof(buf));
   send_buf(sock,&server,buf,sizeof(buf));

   exit (0);

}

int send_buf(int sock,struct sockaddr_in *server,char *buf,int buflen){
  /* Return -1 if error */
  printf("El tamaño de buf es %i\n",buflen);
   if(!(sendto(sock,buf,buflen,0,(struct sockaddr *)server, sizeof(*server)))==buflen){
      return (-1);
   }
   return (0);
}

