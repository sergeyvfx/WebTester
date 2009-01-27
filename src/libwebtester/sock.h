/**
 * WebTester Server - server of on-line testing system
 *
 * Implementation of sockets stuff
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#ifndef _sock_h_
#define _sock_h_

#include <libwebtester/smartinclude.h>

BEGIN_HEADER

#include <sys/socket.h>
#include <sys/un.h>

#define SE_UNKNOWN_HOST     -2
#define SE_CANT_CREATE_SOCK -3
#define SE_CANT_CONNECT     -4

/* Create INET socket for server */
int
sock_create_inet (const char *__host, unsigned int __port);

/* Destroy created socket */
int
sock_destroy (int __sock);

/* Set non-blocking option to socket */
int
sock_set_nonblock (int __sock, int __val);

/* Set noinherit option to socket */
int
sock_set_noinherit (int __sock, int __val);

/* Formatted answer to socket */
int
sock_answer (int __sock, const char* __text, ...);

/* Read buffer from socket */
int
sock_read (int __sock, int __maxlen, char* __out);

/* Create INET socket for client */
int
sock_create_client_inet (const char *__hostname, unsigned int __port);

/* Create UNIX socket for server */
int
sock_create_unix (const char *__fn);

/* Create UNIX socket for client */
int
sock_create_client_unix (const char *__fn);

END_HEADER

#endif
