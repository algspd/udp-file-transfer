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
/*       receive() waits until buffer is full                             */
/**************************************************************************/
int receive(int s,char *rbuffer,int rbufferlen){
  struct sockaddr_in remote;
  int rlen;
  rlen = sizeof(remote);
  memset(rbuffer,0,sizeof(rbuffer));
  if (recvfrom(s,rbuffer,rbufferlen,MSG_WAITALL,(struct sockaddr *)&remote,(socklen_t *)&rlen) < 0) {
    return (-1);
  }
  return(0);
}