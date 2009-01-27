/**
 * WebTester Server - server of on-line testing system
 *
 * Encryptor for IPC passwords
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include <stdio.h>
#include <malloc.h>

#define NO_CONFIG

#include <libwebtester/md5.h>

#define SALT "#RANDOM#"

int
main (int __argc, char **__argv)
{
  char *res = malloc (1024);
  char *salt = SALT;
  if (__argc != 2 && __argc != 3)
    {
      printf ("Usage: %s <pass to encrypt> [<salt>]\n", __argv[0]);
      return -1;
    }

  if (__argc == 3)
    {
      salt = __argv[2];
    }

  md5_crypt (__argv[1], salt, res);
  printf ("%s\n", res + 8);

  free (res);

  return 0;
}
