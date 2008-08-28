/*
 *
 * =============================================================================
 *  file.c
 * =============================================================================
 *
 *  Written (by Nazgul) under GPL
 *
*/

#ifndef _fs_h_
#define _fs_h_

#include <stdio.h>
#include <sys/types.h>

#include <libwebtester/dynastruc.h>

#define FS_COPY_BUF_SIZE 65535

////////////////////////////////////////
//

dynastruc_t*
dir_listing                        (char *__dir);

char*
get_full_path                      (char *__fn);

char*
get_full_file                      (char *__fn);

size_t
fsize                              (FILE *__stream);

char*
fload                              (char *__fn);

int
fexists                            (char *__fn);

int
isfile                             (char *__fn);

int
fmkdir                             (char *__dir, mode_t __mode);

int
unlinkdir                          (char *__dir);

int
copyfile                           (char *__src, char *__dst);

int
fwritebuf                          (char *__fs, char *__buf);

int
unpack_file                        (char *__fn, char *__dstdir);

int
pack_file                          (char *__fn, char *__packer);

int
fcopydir                           (char *__src, char *__dst);

int
fdup                               (char *__fn, char *__out, char *__add_ext, int __count);

#endif
