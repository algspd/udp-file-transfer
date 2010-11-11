/*********************************************************************.****/
/* CS244 file server program -- just returns requested blocks of file     */
/* in local directory.                                                    */
/********************************************************************+*****/

/* Compilar con:
     cc  -lsocket  -lnsl -o sftpd sftpd2.c */


#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "sftp2.h"


int start_server(),process_requests();


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
/*       start up the server: get a socket and test port                  */
/*       returns -1 for reserved or out-of-range port                     */
/*       returns -2 for other errors                                      */
/*       on success returns socket                                        */
/**************************************************************************/

int start_server(int port){
  struct sockaddr_in local;
  int s=-2;

  if ((s = socket(AF_INET,SOCK_DGRAM,0)) < 0) {
    // Couldn't create socket
    if (port > 65535 || port < IPPORT_RESERVED) {
      // Port reserved or out-of-range
      s=-1;
    }
    return(s);
  }

  memset((char *)&local,0,sizeof(local));
  local.sin_family = AF_INET;
  local.sin_port = htons((short)port);
  local.sin_addr.s_addr = INADDR_ANY;

  if ((bind(s,(struct sockaddr *)&local,sizeof(local))) != 0) {
    if (errno != EADDRINUSE) {
      return(-2);
    }
    return(-1);
  }
  return(s);
}

int receive(int s,char *rbuffer,int rbufferlen){
  struct sockaddr_in remote;
  int rlen;
  rlen = sizeof(remote);
  memset(rbuffer,0,sizeof(rbuffer));
  if (recvfrom(s,rbuffer,rbufferlen,0,(struct sockaddr *)&remote,(socklen_t *)&rlen) < 0) {
    return (-1);
  }
  return(0);
}


/**************************************************************************/
/* handle all requests -- does not return unless fatal error              */
/**************************************************************************/

int process_requests(int s){
  char buffer[1000];
  while (1) {
    if(receive(s,buffer,sizeof(buffer))==0){
      printf("Received data:\n%s\n",buffer);
    }
    else{
      printf("Error receiving\n");
    }
  }
}



