/**************************************************************************/
/*     protocol definitions for Stupid File Transfer Protocol (CS244)     */
/**************************************************************************/

#define MAXNAMELEN 64	/* maximum filename length */
#define BUFLEN 512	/* buffer length */

#define SFTP_VERSION    1

#define OP_READ		1
#define OP_READ_REPLY	2

struct request {
    u_char	r_version;		/* Version 1 */
    u_char	r_opcode;		/* OP_READ */
    u_short	r_sum;			/* Checksum */
    u_long	r_fid;			/* file id */
    u_long	r_offset;		/* where to read next block from */
    char	r_filename[MAXNAMELEN];	/* filename */
};

#define A_PERRORS (0x8000)	/* protocol errors have this bit set */
#define A_FORMAT (A_PERRORS|0x1) /* request misformatted */

struct answer {
    u_char	a_version;		/* Version 1 */
    u_char	a_opcode;		/* OP_READ_REPLY */
    u_short	a_sum;			/* Checksum */
    u_long	a_fid;			/* returning the file id */
    u_long	a_offset;		/* where the data came from */
    u_short	a_code;			/* answer code */
    u_short	a_len;			/* how much data in this buffer */
    char	a_buffer[BUFLEN];	/* actual data */
} ;
