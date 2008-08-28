/*
 *
 * ================================================================================
 *  uid.h - part of the WebTester Server
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include <uid.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>

#include <time.h>
#include <sys/time.h>

#include "md5.h"

char*
uid_gen                            (char *__out)
{
  char salt[9]={0}, pass[128]={0}, dummy[1024];
  int i;
  struct timeval tv;

  if (!__out) return 0;
  
  gettimeofday (&tv, 0);
  srand (time (0)+tv.tv_usec);

  for (i=0; i<8; i++)
    salt[i]=rand ()%220+32;

  sprintf (pass, "%ld-%s", (long)time (0), salt);

  md5_crypt (pass, salt, dummy);

  strcpy (__out, dummy+8);

  return __out;
}
