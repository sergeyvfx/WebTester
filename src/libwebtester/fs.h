/**
 * WebTester Server - server of on-line testing system
 *
 * Copyright 2008 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#ifndef _fs_h_
#define _fs_h_

#include "smartinclude.h"

BEGIN_HEADER

#include <stdio.h>
#include <sys/types.h>

#include <libwebtester/dynastruc.h>

#define FS_COPY_BUF_SIZE 65535

/* Get listing of directory */
dynastruc_t*
dir_listing (const char *__dir);

/* Get full path name */
char*
get_full_path (const char *__fn);

/* Get full path to file */
char*
get_full_file (const char *__fn);

/* Get stream size */
size_t
stream_size (FILE *__stream);

/* Get file size */
size_t
fsize (const char *__fn);

/* Load file content */
char*
fload (const char *__fn);

/* Check is file exists */
int
fexists (const char *__fn);

/* Check if specified file name is a regular file */
int
isfile (const char *__fn);

/* Create directory with all parents */
int
fmkdir (const char *__dir, mode_t __mode);

/* Recursively deletion of directory */
int
unlinkdir (const char *__dir);

/* Copy file */
int
copyfile (const char *__src, const char *__dst);

/* Write buffer to file */
int
fwritebuf (const char *__fs, const char *__buf);

/* Unpack file */
int
unpack_file (const char *__fn, const char *__dstdir);

/* Pack file */
int
pack_file (const char *__fn, const char *__packer);

/* Copy directory (recursively) */
int
fcopydir (const char *__src, const char *__dst);

/* Make new itered file name */
int
fdup (const char *__fn, char *__out, const char *__add_ext, int __count);

END_HEADER

#endif
