/**
 * WebTester Server - server of on-line testing system
 *
 * SAMBA network stuff abstraction
 *
 * Copyright 2008 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
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

typedef struct
{
  char *workgroup;
  char *login;
  char *passwd;
} auth_data_t;

static assarr_t *auth_data_cache = NULL;
static auth_data_t def_auth_data = {"", "", ""};
static int initialized = FALSE;

static auth_data_t*
cache_get_auth_data (const char *__server, const char *__share);

/**
 * Get auth. data by server and share
 *
 * @param __server - name of server
 * @param - __share - name of share
 * @return authentificate data
 */
static auth_data_t*
get_auth_data (const char *__server, const char *__share)
{
  auth_data_t *ret = cache_get_auth_data (__server, __share);
  return ((ret) ? (ret) : (&def_auth_data));
}

/**
 * Authontification  function
 *
 * @param __server - name of server
 * @param __share - name of share
 * @param __workgroup - workgroup name
 * @param __max_workgroup_len - max workgroup name length
 * @param __username - user name
 * @param __max_username_len - max user name length
 * @param __password - password
 * @param __max_password_len - max password length
 */
static void
samba_auth_funct (const char *__server, const char *__share,
                  char *__workgroup, int __max_workgroup_len,
                  char *__username, int __max_username_len,
                  char *__password, int __max_password_len)
{
  auth_data_t *auth_data = get_auth_data (__server, __share);

  DEBUG_LOG ("network-smb", "Entering to server: %s, share: %s\n",
             __server, __share);

  if (!auth_data)
    {
      return;
    }

  strcpy (__workgroup, auth_data->workgroup);
  strcpy (__username, auth_data->login);
  strcpy (__password, auth_data->passwd);
}

/**
 * Create new authentification data
 *
 * @param __workgroup - workgroup name
 * @param __login - user's login
 * @param __passwd - user's password
 * @return new authentification data
 */
static auth_data_t*
spawn_new_auth_data (const char *__workgroup, const char *__login,
                     const char *__passwd)
{
  auth_data_t *ptr = malloc (sizeof (auth_data_t));
  ptr->workgroup = strdup (__workgroup);
  ptr->login = strdup (__login);
  ptr->passwd = strdup (__passwd);

  return ptr;
}

/**
 * Deleter for authentification data
 *
 * @param __self - data to delete
 */
static void
auth_data_deleter (void *__self)
{
  auth_data_t *self = __self;

  free (self->workgroup);
  free (self->login);
  free (self->passwd);
  free (self);
}

/**
 * Set default authentifiacation data
 *
 * @param __workgroup - workgroup name
 * @param __login - user's login
 * @param __passwd - user's password
 */
static void
set_default_auth_data (const char *__workgroup, const char *__login,
                       const char *__passwd)
{
  strcpy (def_auth_data.workgroup, __workgroup);
  strcpy (def_auth_data.login, __login);
  strcpy (def_auth_data.passwd, __passwd);
}

/********
 * Cache
 */

/**
 * Create cache
 */
static void
cache_create (void)
{
  auth_data_cache = assarr_create ();
}

/**
 * Push authentification data to cache
 *
 * @param __server - server name
 * @param __share - share name
 * @param __workgroup - workgroup name
 * @param __login - user's login
 * @param __passwd - user's password
 */
static void
cache_push_auth_data (const char* __server, const char *__share,
                      const char *__workgroup, const char *__login,
                      const char *__passwd)
{
  char key[1024];

  DEBUG_LOG ("network-smb", "Push auth. data for server "
          "%s and share %s\n", __server, __share);

  auth_data_t *ptr = spawn_new_auth_data (__workgroup, __login, __passwd);
  BUILD_CACHE_KEY (key, __server, __share);
  assarr_set_value (auth_data_cache, key, ptr);
}

/**
 * Get authentification data from cache
 *
 * @param __server - server name
 * @param __share - share name
 */
static auth_data_t*
cache_get_auth_data (const char *__server, const char *__share)
{
  char key[4096];
  BUILD_CACHE_KEY (key, __server, __share);
  return assarr_get_value (auth_data_cache, key);
}

/**
 * Destroy authentification cache
 */
static void
cache_destroy (void)
{
  assarr_destroy (auth_data_cache, auth_data_deleter);
}

/********
 * User's backend
 */

/**
 * Initialize SAMBA client stuff
 *
 * @return zero on success, non-zero otherwise
 */
int
samba_init (void)
{
  cache_create ();

  smbc_init (samba_auth_funct, FALSE);

  initialized = TRUE;
  return 0;
}

/**
 * Uninitialize SAMBA client stuff
 */
void
samba_done (void)
{
  cache_destroy ();
  initialized = FALSE;
}

/**
 * Is SAMBA client stuff initialized?
 */
int
samba_initialized (void)
{
  return initialized;
}

/**
 * Set default SAMBA cient stuff authentification data
 *
 * @param __workgroup - workgroup name
 * @param __login - user's login
 * @param __passwd - user's password
 */
void
samba_set_default_auth_data (const char *__workgroup, const char *__login,
                             const char *__passwd)
{
  set_default_auth_data (__workgroup, __login, __passwd);
}

/**
 * Push authentification data
 *
 * @param __server - server name
 * @param __share - share name
 * @param __workgroup - workgroup name
 * @param __login - user's login
 * @param __passwd - user's password
 */
void
samba_push_auth_data (const char *__server, const char *__share,
                      const char *__workgroup, const char *__login,
                      const char *__passwd)
{
  cache_push_auth_data (__server, __share, __workgroup, __login, __passwd);
}

/****
 *
 */

/**
 * Open remote file and return descriptor
 *
 * @param __fn - name of file to open
 * @oaram __mode - mode of opening
 * @param __flagc - opening flags
 * @return file's descriptor
 */
int
samba_fopen (const char *__fn, int __mode, int __flags)
{
  return smbc_open (__fn, __mode, __flags);
}

/**
 * Close remote file
 *
 * @param __fd - descriptor of file to close
 */
void
samba_fclose (int __fd)
{
  smbc_close (__fd);
}

/**
 * Read buffer from remote file
 *
 * @param __fd - file descriptor
 * @param __buf - buffer
 * @param __size - size of buffer
 * @return number of read bytes
 */
int
samba_fread (int __fd, char *__buf, __u32_t __size)
{
  return smbc_read (__fd, __buf, __size);
}

/**
 * Unlink remote file
 *
 * @param __fn - name of remote file to unlink
 */
int
samba_unlink (const char *__fn)
{
  return smbc_unlink (__fn);
}
