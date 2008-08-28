/*
 *
 * ================================================================================
 *  network-smb.c
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include "network-smb.h"
#include "assarr.h"
#include "types.h"
#include "macrodef.h"

#include <malloc.h>
#include <string.h>
#include <stdio.h>

#include <libsmbclient.h>

#define BUILD_CACHE_KEY(__key, __server, __share) \
  sprintf (__key, "%s@%s", __server, __share)

typedef struct {
  char *workgroup;
  char *login;
  char *passwd;
} auth_data_t;

static assarr_t *auth_data_cache=NULL;
static auth_data_t def_auth_data={"", "", ""};
static int initialized=FALSE;

////
//

static auth_data_t*
cache_get_auth_data                (const char *__server, const char *__share);

////
//

static auth_data_t* // Get auth. data by server and share
get_auth_data (const char *__server, const char *__share)
{
  auth_data_t *ret=cache_get_auth_data (__server, __share);
  return ((ret)?(ret):(&def_auth_data));
}

static void // Authontification  function
samba_auth_funct                   (const char *__pServer,
                                    const char *__pShare,
                                    char       *__pWorkgroup,
                                    int         __maxLenWorkgroup,
                                    char       *__pUsername,
                                    int         __maxLenUsername,
                                    char       *__pPassword,
                                    int        __maxLenPassword)
{
  auth_data_t *auth_data=get_auth_data (__pServer, __pShare);
  
  DEBUG_LOG ("network-smb", "Entering to server: %s, share: %s\n", __pServer, __pShare);

  if (!auth_data) return;

  strcpy (__pWorkgroup, auth_data->workgroup);
  strcpy (__pUsername,  auth_data->login);
  strcpy (__pPassword,  auth_data->passwd);
}

//////
//

static auth_data_t*
spawn_new_auth_data                (const char *__workgroup, const char *__login, const char *__passwd)
{
  auth_data_t *ptr=malloc (sizeof (auth_data_t));
  ptr->workgroup = strdup (__workgroup);
  ptr->login     = strdup (__login);
  ptr->passwd    = strdup (__passwd);

  return ptr;
}

static void
auth_data_deleter                  (void *__self)
{
  auth_data_t *self=__self;

  free (self->workgroup);
  free (self->login);
  free (self->passwd);
  free (self);
}

static void
set_default_auth_data              (const char *__workgroup, const char *__login, const char *__passwd)
{
  strcpy (def_auth_data.workgroup, __workgroup);
  strcpy (def_auth_data.login,     __login);
  strcpy (def_auth_data.passwd,    __passwd);
}

////////
// Cache

static void
cache_create                     (void)
{
  auth_data_cache=assarr_create ();
}

static void
cache_push_auth_data               (const char* __server, const char *__share, const char *__workgroup, const char *__login, const char *__passwd)
{
  char key[1024];
  DEBUG_LOG ("network-smb", "Push auth. data for server %s and share %s\n", __server, __share);
  auth_data_t *ptr=spawn_new_auth_data (__workgroup, __login, __passwd);
  BUILD_CACHE_KEY (key, __server,  __share);
  assarr_set_value (auth_data_cache, key, ptr);
}

static auth_data_t*
cache_get_auth_data                (const char *__server, const char *__share)
{
  char key[4096];
  BUILD_CACHE_KEY (key, __server, __share); 
  return assarr_get_value (auth_data_cache, key);
}

static void
cache_destroy                      (void)
{
  assarr_destroy (auth_data_cache, auth_data_deleter);
}

/////////////////////////////////////////
// End user's stuff

int // Initialize SAMBA client stuff
samba_init                         (void)
{
  cache_create ();

  smbc_init (samba_auth_funct, FALSE);
  
  initialized=TRUE;
  return 0;
}

void // Uninitialize SAMBA client stuff
samba_done                         (void)
{
  cache_destroy ();
  initialized=FALSE;
}

int
samba_initialized                  (void)
{
  return initialized;
}

void
samba_set_default_auth_data        (const char *__workgroup, const char *__login, const char *__passwd)
{
  set_default_auth_data (__workgroup, __login, __passwd);
}

void
samba_push_auth_data               (const char *__server, const char *__share, const char *__workgroup, const char *__login, const char *__passwd)
{
  cache_push_auth_data (__server, __share, __workgroup, __login, __passwd);
}

////////
//

int
samba_fopen                        (const char *__fn, int __mode, int __flags)
{
  return smbc_open (__fn, __mode, __flags);
}

void
samba_fclose                       (int __fd)
{
  smbc_close (__fd);
}

int
samba_fread                        (int __fd, char *__buf, __u32_t __size)
{
  return smbc_read (__fd, __buf, __size);
}

int
samba_unlink                       (const char *__fn)
{
  return smbc_unlink (__fn);
}
