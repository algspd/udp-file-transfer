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
#include "sftp2.h"

#define ERRORSTRING "unknown error"
#define MAX_REC 10000
#define WINDOW_SIZE 7
#define SELECT_TIMEOUT 1000
#define SELECT_TIMEOUT_S 0


u_short xsum();
int send_request_timeout();
int send_request();
int receive_answers();
int exit_p();


   struct req_s{
      int offset;
      struct timeval req_time;
      struct timeval timeout;
   };

   struct pac_s{
      int received;
      int requested;
      int right;
      int dup;
      int bad_xsum;
      int bad_offset;
      int reasked;
   };
   
   struct time_s{
      struct timeval start;
      struct timeval end;
      
      float select;
      float write;
   };
   
   typedef struct{
      struct req_s req[WINDOW_SIZE];  //Packets requested and still not received
      struct pac_s pac;
      struct time_s time;
      int last_offset;
      int rec[MAX_REC];
      int last_rec;
   }window_s;


struct  timeval  tvSubtract (struct  timeval  time1,struct  timeval  time2){
   struct  timeval  result ;
   /* MUST TIME1 > TIME2 */
   result.tv_sec = time1.tv_sec - time2.tv_sec ;
   if (time1.tv_usec < time2.tv_usec) {
      result.tv_usec = time1.tv_usec + 1000000L - time2.tv_usec ;
      /* Borrow a second. */
      result.tv_sec-- ;
   } else {
      result.tv_usec = time1.tv_usec - time2.tv_usec ;
   }
   return (result) ;
}


double  tvFloat (struct  timeval  time){
    return ((double) time.tv_sec + (time.tv_usec / 1000000.0)) ;
}



int main (int argc,char **argv){
   int fin=0;
   int verbose=0,EOFile=0;
   struct timeval startTime,endTime;
   struct sockaddr_in server;
   struct hostent *host_info;
   char SERVER[MAXNAMELEN],infilename[MAXNAMELEN],outfilename[MAXNAMELEN],*port,*hostname;
   char portatserv[MAXNAMELEN];
   int fd,sock,args=1,SERVER_PORT,i;
   char usage[100]="[-v] get 1024@hendrix01.cps.unizar.es remote_file [local_file]\n\0";
   struct pac_s pac1={0,0,0,0,0,0,0};

   // Window initialization
   window_s window;
   window.pac=pac1;
   window.last_rec=0;
   window.time.select=(double)0;
   
   /* PARSE COMMAND LINE ARGUMENTS */
   if (argc<3){
      fprintf(stderr,"Usage: %s %s",*argv,usage);
       return 1;
   }
   
   /* Test verbose flag */
   if (strcmp(argv[args],"-v") == 0){
      args++;
      if (argc<=4){
         fprintf(stderr,"Usage: %s %s",*argv,usage);
         return 1;
      }
      verbose=1;
   }
   if (strcmp(argv[args++],"get")!=0){
      fprintf(stderr,"Usage: %s %s",*argv,usage);
      return 1;
   }
   /* Parse port@host */
   strcpy(portatserv,argv[args++]);
   port=strtok(portatserv,"@");
   hostname=strtok(NULL,"@");
   SERVER_PORT=atoi(port);
   strcpy(SERVER,hostname);
   /* Parse infile */
   strncpy(infilename,argv[args++],MAXNAMELEN);
   /* Parse outfile */
   strncpy(outfilename,infilename,MAXNAMELEN);
   if (argc>args) {
      strncpy(outfilename,argv[args],MAXNAMELEN);
   }
   
   /* PRINT PARSED ARGUMENTS */
   if(verbose){
      printf("Port %i<--\n",SERVER_PORT);
      printf("Server %s<--\n",SERVER);
      printf("Infile %s<--\n",infilename);
      printf("Outfile %s<--\n\n",outfilename);
   }
   
   /* OPEN/CREATE OUTPUT FILE */
   fd=open(outfilename,O_CREAT+O_WRONLY+O_TRUNC,00777);
   if (fd<0){
      fprintf(stderr,"Couldn't create out file");
      return 1;
   }

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

   window.last_offset=0;
   
   printf("Starting file download\n\n");
   fin=0;

   gettimeofday(&startTime,NULL);
   
   for (i=0;i<WINDOW_SIZE;i++){
      // Empty window
      window.req[i].offset=-1;
   }
   for (i=0;i<MAX_REC;i++){
      // Empty window
      window.rec[i]=-1;
   }

   while (!fin){
      send_request_timeout(sock,infilename,&server,&window,verbose,EOFile);
      if (!EOFile){
         send_request(sock,infilename,&server,&window,verbose);
      }
      fin=receive_answers(sock,&server,fd,&window,&EOFile,verbose);
      
      if(verbose && !EOFile) printf("\n");
   }
   
   if (window.last_offset==0){
      fprintf(stderr,"File transfer failed\n");
   }
   else{
      gettimeofday(&endTime,NULL);
      printf("File transfer successfull\n");
      printf("Transfer time was %f seconds\n",tvFloat(tvSubtract(endTime,startTime)));
      printf("File contains %i characters\n",window.last_offset);
      printf("Transfer speed: %.2f kb/s\n",((double)window.last_offset/1000)/tvFloat(tvSubtract(endTime,startTime)));
   }

   exit_p(0,&window);
   return(0);
}

int send_request_timeout(int sock,char filename[MAXNAMELEN],struct sockaddr_in *server,window_s *window,int verbose,int EOFile){
   struct timeval now;
   struct request *req;
   struct request req2;
   int i,exit_f;
   req=&req2;
   gettimeofday(&now,NULL);
   
   for (i=0;i<WINDOW_SIZE;i++){
      if ((window->req[i].offset >=0) &&
         ((window->req[i].req_time.tv_usec + window->req[i].timeout.tv_usec <= now.tv_usec) || 
         (window->req[i].req_time.tv_sec + window->req[i].timeout.tv_sec <= now.tv_sec ))){

         req->r_version=SFTP_VERSION;
         req->r_opcode=OP_READ;                        /* OP_REined symbol: window */
         req->r_sum=0;                                 /* Checksum */
         req->r_fid=5;                                 /* file id */
         req->r_offset=window->req[i].offset;          /* where to read next block from */
         strncpy(req->r_filename,filename,MAXNAMELEN); /* filename */
         
         req->r_sum=xsum((char *)req,sizeof(*req));

         req->r_version=htons(req->r_version);
         req->r_opcode=htons(req->r_opcode);                        /* OP_REined symbol: window */
         req->r_sum=htons(0);                                 /* Checksum */
         req->r_fid=htons(5);                                 /* file id */
         req->r_offset=htons(req->r_offset);          /* where to read next block from */
         
         if(verbose){
            printf("Sending request:\n");
            printf("\tName:[%s]\n",req->r_filename);
            printf("\tOp: [%d]\n\tVersion: [%d]\n", req->r_opcode, req->r_version);
            printf("\tOffset:[%lu]\n",req->r_offset);
            printf("\tFile ID:[0x%lu]\n",req->r_fid);
            printf("\tsum:[0x%x]\n",req->r_sum);
         }
         if(!(sendto(sock, req, sizeof(*req),0,(struct sockaddr *)server, sizeof(*server)))==sizeof(*req)){
            fprintf(stderr,"Couldn't send request\n");
            return (-1);
         }
         window->pac.reasked++;
         window->req[i].timeout.tv_sec = 1;
         window->req[i].timeout.tv_usec = window->req[i].timeout.tv_usec*2;
         gettimeofday(&window->req[i].req_time,NULL);
      }
   }
   exit_f=1;
   for (i=0;i<WINDOW_SIZE;i++){
      if (window->req[i].timeout.tv_usec<5000){
         // Timeout limit exceeded for all packets
         exit_f=0;
      }
   }
   if (exit_f && !EOFile){
      fprintf(stderr,"ERROR: Server not responding\n");
      exit_p(1,window);
   }

   return (0);
}

int send_request(int sock,char filename[MAXNAMELEN],struct sockaddr_in *server,window_s *window,int verbose){
   struct request *req;
   struct request req2;
   
   int i,pos=0;
   req=&req2;
   while (pos>=0){
      // Send requests to fill the window
      pos=-1;
      for (i=0;i<WINDOW_SIZE;i++){
         if (window->req[i].offset==-1){
            pos=i;
            i=WINDOW_SIZE;
         }
      }
      
      if(pos>=0){
         req->r_version=SFTP_VERSION;
         req->r_opcode=OP_READ;
         req->r_sum=0;
         req->r_fid=5;
         req->r_offset=window->last_offset;
         strncpy(req->r_filename,filename,MAXNAMELEN);
         
         req->r_sum=xsum((char *)req,sizeof(*req));
         if(verbose){
            printf("Sending request:\n");
            printf("\tName:[%s]\n",req->r_filename);
            printf("\tOp: [%d]\n\tVersion: [%d]\n", req->r_opcode, req->r_version);
            printf("\tOffset:[%lu]\n",req->r_offset);
            printf("\tFile ID:[0x%lu]\n",req->r_fid);    
            printf("\tsum:[0x%x]\n",req->r_sum);
         }
         if(!(sendto(sock, req, sizeof(*req),0,(struct sockaddr *)server, sizeof(*server)))==sizeof(*req)){
            fprintf(stderr,"Couldn't send request\n");
            return (-1);
         }
         window->pac.requested++;
         window->req[pos].offset=window->last_offset;
         window->req[pos].timeout.tv_sec = 0;
         window->req[pos].timeout.tv_usec = 1000;
         gettimeofday(&window->req[pos].req_time,NULL);
         window->last_offset+=BUFLEN;
      }
   }
   return(0); 

}

int check_xsum(struct answer ans){
   u_short sum;
   int ret_val;
   sum=ans.a_sum;
   ans.a_sum=0;
   ret_val=(xsum((char *)&ans,sizeof(ans)-BUFLEN+ans.a_len) == sum);
   return(ret_val);
}

int check_dup(struct answer ans,window_s *window){
   int i;
   for (i=window->last_rec;i>0;i--){
      if (window->rec[i]==ans.a_offset){
         return(0);
      }
   }
   return(1);
}
   
   
int receive_answers(int sock,struct sockaddr server,int fd,window_s *window,int *EOFile,int verbose){
   static struct answer ans;
   fd_set readfds, readyfds;
   int slen,ansval,longbuf,i,nothing_to_read=0;
   char buf[BUFLEN];
   
   struct timeval timeout;
   timeout.tv_sec = SELECT_TIMEOUT_S;
   timeout.tv_usec = SELECT_TIMEOUT;

   slen = sizeof(server);
   FD_ZERO(&readfds);
   FD_SET(sock, &readfds);
   readyfds = readfds;
   

   if (!nothing_to_read){
      gettimeofday(&window->time.start,NULL);
      if (select(sock+1,&readyfds,NULL,NULL,&timeout) == -1){
         fprintf(stderr, "Select error: %s.\n", ERRORSTRING);
      }
      gettimeofday(&window->time.end,NULL);
      window->time.select+=tvFloat(tvSubtract(window->time.end,window->time.start));
      
      
      if (FD_ISSET(sock, &readyfds) != 0){
         ansval = recvfrom(sock,(char *)&ans,sizeof(ans),0,(struct sockaddr *)&server,&slen);
         if (ansval < 0) {
            fprintf(stderr,"recvfrom: %s\n",ERRORSTRING);
            return (-1);
         }
         window->pac.received++;
         longbuf=ans.a_len;
         memcpy(buf,ans.a_buffer,BUFLEN);
         if ((check_xsum(ans)) == 0){
            // If bad checksum
            longbuf=-1;
            window->pac.bad_xsum++;
         }
         if (longbuf>=0){
            if(!(check_dup(ans,window) && longbuf>=0)){
               // If dup or not requested
               longbuf=-1;
               window->pac.dup++;
            }
         
            if (longbuf==0){
               *EOFile=1;
               for (i=WINDOW_SIZE-1;i>=0;i--){
                  if (window->req[i].offset==ans.a_offset){
                     window->req[i].offset=-1;
                  }
               }
               return (0);
            }
            if (longbuf>0){
               if(verbose) printf("Correct answer received, writing to file\n");
               lseek(fd,ans.a_offset,SEEK_SET);
               if (write(fd,buf,longbuf)!=ans.a_len){
                  printf("ERROR: long no coincide\n");
               }
               window->rec[window->last_rec++]=ans.a_offset;
               window->pac.right++;
               if(verbose) printf("Packet written to file with offset %i\n",ans.a_offset);
               for (i=WINDOW_SIZE-1;i>=0;i--){
                  if (window->req[i].offset==ans.a_offset){
                     window->req[i].offset=-1;
                  }
               }
            }
            nothing_to_read=1;
            for (i=0;i<WINDOW_SIZE;i++){
               if (window->req[i].offset!=-1){
                  nothing_to_read=0;
               }
            }
         }
      }
      else{
         nothing_to_read=1;
      }
   }
   if (*EOFile){
      for (i=0;i<WINDOW_SIZE;i++){
         if (window->req[i].offset!=-1){
            return(0);
         }
      }
      return(1);
   }
   return (0);
}

   
int exit_p(int err_out,window_s *window){
   printf("\nPackets:\n");
   printf("Requested:\t %i\n",window->pac.requested);
   printf("Rerequests:\t %i\n",window->pac.reasked);
   printf("Total Requests:\t %i\n\n",window->pac.requested+window->pac.reasked);
   
   printf("Received:\t %i\n",window->pac.received);
   printf("Without errors:\t %i\n",window->pac.right);
   
   printf("\nDuplicated:\t %i\n",window->pac.dup);
   printf("Bad checksum:\t %i\n",window->pac.bad_xsum);
   printf("Packets lost:\t %i\n",(window->pac.requested+window->pac.reasked)-window->pac.received+window->pac.dup);
   
   printf("Select time : %f s\n",window->time.select);
   
   
   exit(err_out);
}


u_short
xsum(buf,len)
     char		*buf;
     int		len;
{
  register u_short	*sp;
  register int	slen;
  register u_long	sum;		/* >= 32-bit space to keep sum */
  union { u_short s; u_char c[2]; }
  			xun;
  int			unaligned;

  sum = 0;

  unaligned = ((unsigned long)buf) & 0x1;
  /* If buffer isn't short aligned, need to get fisst byte */
  if (unaligned != 0) {
    xun.s = 0;
    xun.c[1] = buf[0];
    sum = xun.s;
    buf++; len--;
  } 
  slen = len/2;			/* length in shorts */

  /* LINT NOTE: next line has possible ptr alignment message, even 
     though we've made sure that buf is aligned */
  for(sp = (u_short *)buf; slen > 0; slen--,sp++) {
    sum += *sp;
  }

  /* is there a trailing odd byte? */
  if ((len & 0x1) != 0) { 
#ifdef DEBUG
      printf("xsum extra: 0x%X (sum: 0x%X, len %d.)\n", buf[len-1], sum, len); 
#endif
    xun.s = 0; xun.c[0] = buf[len - 1];
    sum += xun.s; 
  }

  /* Fold in all the carries to get a single 16 bit value */
  sum = (sum & 0xFFFF) + (((unsigned long)(sum & 0xFFFF0000))>>16);
  if (sum > 0xFFFF) 
    { sum = (sum & 0xFFFF) + 1; }

  if (unaligned != 0)          /* byteswap */
    { sum = ((sum & 0xFF)<<8) + ((sum & 0xFF00)>>8); }
  return (sum);
}



