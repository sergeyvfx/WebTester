/*
 * ================================================================================
 *  genpass.c - masked random password generator
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License. 02.08.2007
 *
*/

#include <stdio.h>
#include <malloc.h>
#include <libwebtester/md5.h>

#define SALT "#RANDOM#"

int
main                              (int __argc, char **__argv)
{
  char *res=malloc (1024);
  if (__argc!=2)
    {
      printf ("Usage: %s <pass to encrypt>\n", __argv[0]);
      return -1;
    }
  
  md5_crypt (__argv[1], SALT, res);
  printf ("%s\n", res+8);
  
  free (res);
  
  return 0;
}
