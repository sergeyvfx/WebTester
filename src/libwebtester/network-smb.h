/*
 *
 * ================================================================================
 *  network-smb.h
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/


#ifndef _network_smb_h_
#define _network_smb_h_

#include <libwebtester/types.h>
#include <libsmbclient.h>

int // Initialize SAMBA client stuff
samba_init                         (void);

void // Uninitialize SAMBA client stuff
samba_done                         (void);

int
samba_initialized                  (void);

void
samba_set_default_auth_data (const char *__workgroup, const char *__login, const char *__passwd);

void
samba_push_auth_data (const char *__server, const char *__share, const char *__workgroup, const char *__login, const char *__passwd);

////////
//

int
samba_fopen (const char *__fn, int __mode, int __flags);

void
samba_fclose (int __fd);

int
samba_fread (int __fd, char *__buf, __u32_t __size);

int
samba_unlink (const char *__fn);

#endif
