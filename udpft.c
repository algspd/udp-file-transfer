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

#include <string.h>

#include "udpft.h"
#include "md5.h"

int packetType(char *p){
   return((int)*p);
}

/* FGETINFO */
struct fgetinfo *get_info(char file_path[FILENAME_MAX]){
   char *md5;
   struct fgetinfo *p;
   p=malloc(sizeof(struct fgetinfo));
   p->type=1;
   strncpy(p->file_path,file_path,FILENAME_MAX);
   // set md5 to 0
   memset(p->md5,0,sizeof(p->md5));
   strncpy(p->md5,calculate_md5(&p,sizeof(p)),MD5_SIZE);
   return(p);
}

void print_fgetinfo(struct fgetinfo p){
   printf("Packet:\n - type: %c\n",p.type+'0');
   printf(" - path: %s\n",p.file_path);
   printf(" - md5: %s\n",p.md5);
}


/* FGETFRAG */
struct fgetfrag *get_frag(fid_t file_id,off_t offset){
   char *md5;
   struct fgetfrag *p;
   p=malloc(sizeof(struct fgetfrag));
   p->type=2;
   p->file_id=file_id;
   p->offset=offset;
   // set md5 to 0
   memset(p->md5,0,sizeof(p->md5));
   strncpy(p->md5,calculate_md5(&p,sizeof(p)),MD5_SIZE);
   return(p);
}

void print_fgetfrag(struct fgetfrag p){
   printf("Packet:\n - type: %c\n",p.type+'0');
   printf(" - file id: %i\n",p.file_id);
   printf(" - offset: %i\n",p.offset);
   printf(" - md5: %s\n",p.md5);
}

/* FEND */

struct fend *get_fend(fid_t file_id){
   char *md5;
   struct fend *p;
   p=malloc(sizeof(struct fend));
   p->type=3;
   p->file_id=file_id;
   // set md5 to 0
   memset(p->md5,0,sizeof(p->md5));
   strncpy(p->md5,calculate_md5(&p,sizeof(p)),MD5_SIZE);
   return(p);
}

void print_fget_fend(struct fgetfrag p){
   printf("Packet:\n - type: %c\n",p.type+'0');
   printf(" - file id: %i\n",p.file_id);
   printf(" - md5: %s\n",p.md5);
}

/* FINFO */

struct finfo *get_sinfo(int file_exist,int file_id,int file_size){
   char *md5;
   struct finfo *p;
   p=malloc(sizeof(struct finfo));
   p->type=4;
   p->file_exist=file_exist;
   p->file_id=file_id;
   p->file_size=file_size;
   // set md5 to 0
   memset(p->md5,0,sizeof(p->md5));
   strncpy(p->md5,calculate_md5(&p,sizeof(p)),MD5_SIZE);
   return(p);
}
void print_finfo(struct finfo p){
   printf("Packet:\n - type: %c\n",p.type+'0');
   printf(" - file exist: %i\n",p.file_exist);
   printf(" - file id: %i\n",p.file_id);
   printf(" - file size: %i\n",p.file_size);
   printf(" - md5: %s\n",p.md5);
}

/* FFRAG */

struct ffrag *get_ffrag(fid_t file_id,off_t offset,fra_t fragment){
   char *md5;
   struct ffrag *p;
   p=malloc(sizeof(struct ffrag));
   p->type=5;
   p->file_id=file_id;
   p->offset=offset;
   strncpy(p->fragment,fragment,FRAGMENT_SIZE);
   // set md5 to 0
   memset(p->md5,0,sizeof(p->md5));
   strncpy(p->md5,calculate_md5(&p,sizeof(p)),MD5_SIZE);
   return(p);
}
void print_ffrag(struct ffrag p){
   printf("Packet:\n - type: %c\n",p.type+'0');
   printf(" - file id: %i\n",p.file_id);
   printf(" - offset: %i\n",p.offset);
   printf(" - fragment: Better not to print that\n");
   printf(" - md5: %s\n",p.md5);
}

