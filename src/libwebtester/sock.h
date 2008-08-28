/*
 *
 * ================================================================================
 *  sock.h
 * ================================================================================
 *
 *  Sockets stuff
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#ifndef sock_h
#define sock_h

#include <sys/socket.h>
#include <sys/un.h>

#define SE_UNKNOWN_HOST     -2
#define SE_CANT_CREATE_SOCK -3
#define SE_CANT_CONNECT     -4

int
sock_create_inet                   (char *__host, unsigned int __port);

int
sock_destroy                       (int __sock);

int
sock_set_nonblock                  (int __sock, int __val);

int
sock_set_noinherit                 (int __sock, int __val);

int
sock_answer                        (int __sock, char* __text, ...);

int
sock_read                          (int __sock, int __maxlen, char* __out);

int
sock_create_client_inet            (char *__hostname, unsigned int __port);

int
sock_create_unix                   (char *__fn);

int
sock_create_client_unix           (char *__fn);

#endif
