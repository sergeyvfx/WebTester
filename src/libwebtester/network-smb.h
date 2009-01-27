/**
 * WebTester Server - server of on-line testing system
 *
 * SAMBA network stuff abstraction
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#ifndef _network_smb_h_
#define _network_smb_h_

#include <libwebtester/smartinclude.h>

BEGIN_HEADER

#include <libwebtester/types.h>
#include <libsmbclient.h>

/* Initialize SAMBA client stuff */
int
samba_init (void);

/* Uninitialize SAMBA client stuff */
void
samba_done (void);

/* Is SAMBA client stuff initialized? */
int
samba_initialized (void);

/* Set default SAMBA cient stuff authentification data */
void
samba_set_default_auth_data (const char *__workgroup, const char *__login,
                             const char *__passwd);

/* Push authentification data */
void
samba_push_auth_data (const char *__server, const char *__share,
                      const char *__workgroup, const char *__login,
                      const char *__passwd);

/****
 *
 */

/* Open remote file and return descriptor */
int
samba_fopen (const char *__fn, int __mode, int __flags);

/* Close remote file */
void
samba_fclose (int __fd);

/* Read buffer from remote file */
int
samba_fread (int __fd, char *__buf, __u32_t __size);

/* Unlink remote file */
int
samba_unlink (const char *__fn);

END_HEADER

#endif
