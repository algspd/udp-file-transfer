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

#include <stdio.h>
#include <stdlib.h>
#define FRAGMENT_SIZE 1000

typedef char typ_t[1];
typedef char md5_t[33];
typedef int  fid_t;
typedef int  fsi_t;
typedef int  fex_t;
typedef char fra_t[FRAGMENT_SIZE];

/* CLIENT */
struct fgetinfo{
   typ_t type;
   char  file_path[FILENAME_MAX];
   md5_t md5;
};

struct fgetfrag{
   typ_t type;
   fid_t file_id;
   off_t offset;
   md5_t md5;
};

struct fend{
   typ_t type;
   fid_t file_id;
   md5_t md5;
};

/* SERVER */

struct finfo{
   typ_t type;
   fex_t file_exist;
   fid_t file_id;
   fsi_t file_size;
   md5_t md5;
};

struct ffrag{
   typ_t type;
   fid_t file_id;
   off_t offset;
   fra_t fragment;
   md5_t md5;
};