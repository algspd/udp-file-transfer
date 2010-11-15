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

/**************************************************************************/
/*                               defines                                  */
/**************************************************************************/

/* flags */
#define F_VERBOSE       0x1     /* verbose */
#define F_FUNKY         0x2     /* do strange things */
#define F_EXIT          0x4     /* exit in the middle of the transfer */
#define F_MONITOR       0x8     /* print out usage info on exit */

int error_frequency = 5; /* 1 out of error_frequency packets    *
                          * should get an error                 *
                          */
int req_count = 0; /* Counter for bookkeeping */


/* misc stuff */
#define ERRORSTRING ((errno < sys_nerr) ? sys_errlist[errno] : "unknown error")

/* ways to screw up data */
#define E_OFFSET_LO 0
#define E_OFFSET_HI 1
#define E_SUM 2
#define E_KILL 3
#define E_FID 4
#define E_LEN 5
#define E_LEN2 6
#define E_SUMSWAP 7
#define E_EXTRA 8
#define E_MAX 9

/***********************+**************************************************/
/*                            externs                                     */
/**************************************************************************/

//int                   errno;
extern int              sys_nerr;
extern char             *sys_errlist[];


void            process_requests(), print_request();
int             start_server(), init(), fill_abuffer();
u_short         xsum();
int mutilate_packet();

//extern int            fprintf(), printf(), srand();
//extern int            memset();
//extern long           rand();
//extern time_t         time();

/**************************************************************************/
/* MAIN                                                                   */
/**************************************************************************/

int main(argc,argv)
     int                argc;
     char               **argv;
{
    register int        s;
    unsigned            flags;

    if (init(argc,argv,&flags) != 0)
        { return(1); }

    /* start server up */
    if ((s = start_server(flags)) < 0)
        { return(1); }

    /* process requests only returns on errors */
    process_requests(s,flags);

    (void) fprintf(stderr,"sftp server exiting!\n");
    return(1);
}

/**************************************************************************/
/* init -- read flags, set flags bits and seed random number generator    */
/**************************************************************************/

int init(argc,argv,flags)
int             argc;
char            **argv;
unsigned        *flags;
{
    char *progname = *argv;

    *flags = 0;

    for(argc--,argv++; argc > 0; argc--,argv++) {
        if (**argv == '-') {
            switch (*(++*argv)) {
                case 'v':
                    *flags |= F_VERBOSE;
                    break;

                case 'R':
                    error_frequency = atoi(++*argv);
                    *flags |= F_FUNKY;
                    break;

                case 'r':
                    error_frequency = atoi(++*argv);
                    *flags |= F_FUNKY;
                    /* seed random number generator */
                    (void) srand((int) time((time_t *)0));
                    break;
                
                  case 'x':
                    *flags |= F_EXIT;
                    break;

                  case 'm':
                    *flags |= F_MONITOR;
                    break;

                default:
                    goto usage;
            }
        }
        else { goto usage; } 
    }
    if (error_frequency == 0) { error_frequency = 5; }
    return(0);
usage:
    (void) fprintf(stderr,"usage: %s [-v] [-r[freq]] [-R[freq]]\n",progname);
    return(1);
}

/**************************************************************************/
/*       start up the server: get a socket and find a port for it         */
/**************************************************************************/

int start_server(flags)
unsigned                        flags;
{
  register int                  s;
  static struct sockaddr_in     local;

#ifdef lint
  flags = flags;                /* currently no use of flags in this routine */
#endif

  if ((s = socket(AF_INET,SOCK_DGRAM,0)) < 0) {
    (void) fprintf(stderr,"socket: %s\n",ERRORSTRING);
    return(-1);
  }

  memset((char *)&local,0,sizeof(local));
  local.sin_family = AF_INET;
  local.sin_port = htons((short)IPPORT_RESERVED);
  local.sin_addr.s_addr = INADDR_ANY; /* all interfaces */

  /*
   * loop until you find a free port or loop through all the ports
   * or an error other than EADDRINUSE occurs.
   */

  while ((bind(s,(struct sockaddr *)&local,sizeof(local))) != 0) {
    if (errno != EADDRINUSE) {
      (void) fprintf(stderr,"can't bind socket:%s",ERRORSTRING);
      return(-1);
    }
    local.sin_port = htons(ntohs(local.sin_port) + 1);

    if (ntohs(local.sin_port) == IPPORT_RESERVED) {
      (void) fprintf(stderr,"no ports available!\n");
      return(-1);
    }
  }

  (void) printf("sftp server on port %d\n",ntohs(local.sin_port));
  (void) fflush(stdout);
  return(s);
}

/**************************************************************************/
/* handle all requests -- does not return unless fatal error              */
/**************************************************************************/

void process_requests(s,flags)
register int                    s;
unsigned                        flags;
{
  register int          retval;
  static struct sockaddr_in     remote;
  static struct request rbuffer;
  static struct answer  abuffer;
  int                           sendlen;
  int                           rlen;

  while (1) {
    rlen = sizeof(remote);
    retval = recvfrom(s,(char *)&rbuffer,sizeof(rbuffer),0,
                      (struct sockaddr *)&remote,&rlen);

    /* die on error */
    if (retval < 0) {
      (void) fprintf(stderr,"recvfrom: %s\n",ERRORSTRING);
      return;
    }

    req_count++;

    /* print request if in verbose mode */
//     if (flags & F_VERBOSE)
//       { print_request(&rbuffer,retval,&remote); }
// 
//     if ((req_count == 20) && (flags & F_EXIT)) 
//       { return; }
// 
//     /* fill in answer */
//     if (fill_abuffer(&rbuffer,retval,&abuffer,&sendlen,flags) != 0)
//       { continue; }
// 
//     if (mutilate_packet(s, &abuffer, &sendlen, &remote, flags) != 0)
//       { continue; }

    /* ship it */
    if (sendto(s,(char *)&abuffer,sendlen,0,(struct sockaddr *)&remote,
               sizeof(remote)) != sendlen) {
      (void) fprintf(stderr,"sendto(%s[%d]): %s\n",
                     inet_ntoa(remote.sin_addr),ntohs(remote.sin_port),
                     ERRORSTRING);
      return;
    }
  }
  /*NOTREACHED*/
}

/**************************************************************************/
/* print a request                                                        */
/**************************************************************************/

void print_request(rbp,len,sin)
     struct request     *rbp;
     int                len;
     struct sockaddr_in *sin;
{
  struct hostent        *he;

  he = gethostbyaddr((char *)&(sin->sin_addr),4,AF_INET);
  /* inet_ntoa() gives string version of address */
  (void) printf("reqwest from %s:\n",
                (he?he->h_name:inet_ntoa(sin->sin_addr)));
  if (len < sizeof(*rbp)) {
    (void) printf("request too small (%d bytes, should be %d bytes)\n",
                  len,sizeof(*rbp));
    return;
  }
  (void) printf("\tName:[%s]\n",rbp->r_filename);
  (void) printf("\tOp: [%d] Version: [%d]\n", rbp->r_opcode, rbp->r_version);
  (void) printf("\tOffset:[%d]\n",ntohl(rbp->r_offset));
  (void) printf("\tFile ID:[0x%x]\n",ntohl(rbp->r_fid));
  (void) printf("\tsum:[0x%x]\n",ntohs(rbp->r_sum));
}

/**************************************************************************/
/* mutilate a packet to simulate network trouble                          */
/*******************************************************************+******/
int mutilate_packet(s, abp, alenp, remote, flags)
int                     s;
struct answer           *abp;
int                     *alenp;
struct sockaddr_in      *remote;
unsigned                flags;
{

  int                   alen = *alenp;
  /*
   * OK, we now have our data, see if we should mutilate it...
   */

  if ((flags & F_FUNKY) && ((rand() % error_frequency) == 0)) {
    switch (rand() % E_MAX) {

    case E_OFFSET_LO:
      /* give too low offset */
      if (abp->a_offset != 0)
        { abp->a_offset = htonl(ntohl(abp->a_offset)-BUFLEN); }
      if (flags & F_VERBOSE)
        { (void) printf("sending low offset\n"); }
      break;

    case E_OFFSET_HI:
      /* give too high offset */
      abp->a_offset = htonl(ntohl(abp->a_offset)+BUFLEN);
      if (flags & F_VERBOSE)
        { (void) printf("sending high offset\n"); }
      break;

    case E_SUM:
      /* damage sum */
      abp->a_sum ^= (abp->a_sum + 1);
      if (flags & F_VERBOSE)
        { (void) printf("sending damaged sums\n"); } 
      break;

    case E_KILL:
      /* don't reply */
      if (flags & F_VERBOSE)
        { (void) printf("killing packet\n"); }
      return(1);

    case E_FID:
      /* screw up fid */
      abp->a_fid ^= (abp->a_fid + 1);
      if (flags & F_VERBOSE)
        { (void) printf("sending bad fid\n"); }
      break;

    case E_LEN:
      /* make too big */
      abp->a_len = htons(sizeof(*abp) + 1024);
      if (flags & F_VERBOSE)
        { (void) printf("sending long length\n"); }
      break;

    case E_LEN2:
      /* make data too small */
      (*alenp) -= 3;
      if (flags & F_VERBOSE)
        { (void) printf("sending truncated data\n"); }
      break;

    case E_SUMSWAP:
      /* invert checksum */
      {
        union { u_short s; char c[2]; }
        xun;
        char            c;

        xun.s = abp->a_sum;
        c = xun.c[0];
        xun.c[0] = xun.c[1];
        xun.c[1] = c;
        abp->a_sum = xun.s;
      }
      if (flags & F_VERBOSE)
        { (void) printf("sending scrambled sum\n"); }
      break;

    case E_EXTRA:
      /* send extra packet */
      sendto(s,(char *)abp,alen,0,(struct sockaddr *)remote,
             sizeof(*remote));
      if (flags & F_VERBOSE)
        { (void) printf("sending extra packet\n"); }
      /* Maybe corrupt the second copy */
      if ((rand() % 2) == 0) {
        abp->a_sum ^= (abp->a_sum + 1);
        if (flags & F_VERBOSE)
          { (void) printf("sending damaged copy\n"); } 
      }

      break;

      }
  } else if (flags & F_VERBOSE)
    { (void) printf("sending valid packet|n"); }
  return(0);

}

/**************************************************************************/
/* fill in answer buffer based on request                                 */
/**************************+*********************************************+*/

int fill_abuffer(rbp,rlen,abp,alen,flags)
struct request          *rbp;
struct answer           *abp;
int                     rlen, *alen;
unsigned                flags;
{
  register int  i, f;
  int                   retval;
  u_short               save_sum, our_sum;

  /* we cannot reply to because data isn't present */
  if (rlen < sizeof(*rbp)) {
    if (flags & F_VERBOSE)
      { (void) printf("truncated request\n"); }
    return(1);
  }

  save_sum = rbp->r_sum;
  rbp->r_sum = 0;

  /* data is corrupted, again, cannot reply  */
  if ((our_sum = xsum((char *)rbp,sizeof(*rbp))) != save_sum) {
    if (flags & F_VERBOSE) {
      (void) printf("request has bad checksum (0x%x, should be 0x%x)\n",
                    save_sum,our_sum);
    }
    return(1);
  }

  /* prepare to reply */
  memset((char *)abp,0,sizeof(*abp));
  abp->a_version = SFTP_VERSION;
  abp->a_opcode = OP_READ_REPLY;
  abp->a_code = htons(A_FORMAT); /* assume an error */
  abp->a_fid = rbp->r_fid;
  abp->a_offset = rbp->r_offset;
  abp->a_len = 0;
  *alen = sizeof(*abp) - BUFLEN;

  /* Check for malformed packet */
    
  if (rbp->r_version != SFTP_VERSION) {
    if (flags & F_VERBOSE)
      { (void) printf("invalid version %d\n", rbp->r_version); }
    abp->a_sum = xsum((char *)abp,*alen);
    return(0);
  }

  if (rbp->r_opcode != OP_READ) {
    if (flags & F_VERBOSE)
      { (void) printf("unexpected opcode %d\n", rbp->r_opcode); }
    abp->a_sum = xsum((char *)abp,*alen);
    return(0);
  }

  /*
   * check against forgetting the null or other 
   * badness in request file name
   */

  for (i=0; i < MAX_NAMELEN; i++) {
    if (rbp->r_filename[i] == '\0')
      { break;} 
    if (rbp->r_filename[i] == '/') {
      if (flags & F_VERBOSE)
        { (void) printf("invalid char in filename\n"); }
      abp->a_sum = xsum((char *)abp,*alen);
      return(0);
    }
  }


  if (i == MAX_NAMELEN) {
    if (flags & F_VERBOSE)
      { (void) printf("unterminated filename\n"); }
    abp->a_sum = xsum((char *)abp,*alen);
    return(0);
  }

  /* open the requested file */
  if ((f = open(rbp->r_filename,O_RDONLY,0)) < 0) {
    if (flags & F_VERBOSE) {
      (void) printf("open of %s failed, %s\n",
                    rbp->r_filename,ERRORSTRING);
    }
    goto fail;
  }

  /* go to offset */
  rbp->r_offset = ntohl(rbp->r_offset);
  if (lseek(f,(off_t)(rbp->r_offset),SEEK_SET) != rbp->r_offset) {
    if (flags & F_VERBOSE) {
      (void) printf("seek in %s to %d failed: %s\n",
                    rbp->r_filename, rbp->r_offset, ERRORSTRING);
    }
    (void) close(f);
    goto fail;
  }

  /* read it */
  retval = read(f,abp->a_buffer,BUFLEN);
    
  (void) close(f);
  if (retval < 0) {
    if (flags & F_VERBOSE) {
      (void) printf("read in %s at %d failed: %s\n",
                    rbp->r_filename, rbp->r_offset, ERRORSTRING);
    }
    goto fail;
  }

  if ((flags & F_MONITOR) && (retval < BUFLEN)) {
    /* Must be EOF */
    (void) printf("read last block of %s, after %d requests\n", 
                  rbp->r_filename, req_count);
    (void) fflush(stdout);
  }
  
  abp->a_len = htons((short)retval);
  abp->a_code = 0;              /* valid */
  *alen = retval + sizeof(*abp) - BUFLEN;
  abp->a_sum = xsum((char *)abp,*alen);

  return(0);

 fail:
  /* return errno */
  abp->a_code = htons((errno==0?sys_nerr:errno));
  abp->a_sum = xsum((char *)abp,*alen);
  return(0);
}


/**************************************************************************/
/*                     compute a 16-bit sum with carry                    */
/*   nice feature of sum with carry is that it is byte order independent  */
/*       carry from hi-bit of one byte goes into lo-bit of other byte     */
/*   NOTE: assumes data in buf is in net format                           */
/**************************************************************************/

u_short
xsum(buf,len)
     char               *buf;
     int                len;
{
  register u_short      *sp;
  register int  slen;
  register u_long       sum;            /* >= 32-bit space to keep sum */
  union { u_short s; u_char c[2]; }
                        xun;
  int                   unaligned;

  sum = 0;

  unaligned = ((unsigned long)buf) & 0x1;
  /* If buffer isn't short aligned, need to get fisst byte */
  if (unaligned != 0) {
    xun.s = 0;
    xun.c[1] = buf[0];
    sum = xun.s;
    buf++; len--;
  } 
  slen = len/2;                 /* length in shorts */

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
