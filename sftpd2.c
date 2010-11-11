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
#include "udp.h"


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



