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

struct fgetinfo *get_info(char file_path[FILENAME_MAX]){
   char *md5;
   struct fgetinfo *p;

   p=malloc(sizeof(struct fgetinfo));
   p->type='a';
   strncpy(p->file_path,file_path,FILENAME_MAX);
   memset(p->md5,0,sizeof(p->md5));
   md5=calculate_md5(&p,sizeof(p));
   strncpy(p->md5,md5,MD5_SIZE);
   return(p);
}