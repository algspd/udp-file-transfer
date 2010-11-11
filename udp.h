#ifndef UDP_H
   #define UDP_H
   #include <errno.h>
   #include <string.h>
   #include <arpa/inet.h>
#endif
#ifndef PORT_LAST
   #define PORT_LAST 65535
#endif

extern int send_buf(int sock,struct sockaddr_in *server,char *buf,int buflen);
extern int start_server(int port);
extern int receive(int s,char *rbuffer,int rbufferlen);