extern int send_buf(int sock,struct sockaddr_in *server,char *buf,int buflen);
extern int start_server(int port);
extern int receive(int s,char *rbuffer,int rbufferlen);