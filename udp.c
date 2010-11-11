#include "udp.h"


/**************************************************************************/
/*       send buffer: write data from a buffer into a socket              */
/*       returns  0 if no error                                            */
/*       returns -1 for reserved or out-of-range port                     */
/**************************************************************************/
int send_buf(int sock,struct sockaddr_in *server,char *buf,int buflen){
   if(!(sendto(sock,buf,buflen,0,(struct sockaddr *)server, sizeof(*server)))==buflen){
      /* Return -1 if error */
      return (-1);
   }
   return (0);
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
    if (port > PORT_LAST || port < IPPORT_RESERVED) {
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

/**************************************************************************/
/*       receive data from socket                                         */
/*       write it into buffer from 0 to rbufferlen                        */
/*  FIXME: If buffer is too small, data beyond buffer limit is lost WITHOUT ERROR */
/**************************************************************************/
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