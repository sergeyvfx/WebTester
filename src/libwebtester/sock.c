/**
 * WebTester Server - server of on-line testing system
 *
 * Implementation of sockets stuff
 *
 * Copyright 2008 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include "sock.h"
#include "smartinclude.h"

#include <arpa/inet.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <stdio.h>
#include <netdb.h>
#include <fcntl.h>
#include <strings.h>

/* Size of bufefr */
#define BUFSIZE 65536

/**
 * Create INET socket for server
 *
 * @param __host - host name or IP addres to bind to. Use `*` to bind socket
 * to all interfaces.
 * @param __port - port to bind to
 * @return socket on success, -1 otherwise
 */
int
sock_create_inet (const char *__host, unsigned int __port)
{
  int sock;
  int sock_opt = 1;
  struct sockaddr_in name;

  sock = socket (PF_INET, SOCK_STREAM, 0);

  if (sock < 0)
    {
      /* Some troubles */
      return -1;
    }

  name.sin_family = AF_INET;
  name.sin_port = htons (__port);

  if (__host == 0 || !strcmp (__host, "") || !strcmp (__host, "*"))
    {
      name.sin_addr.s_addr = htonl (INADDR_ANY);
    }
  else
    {
      name.sin_addr.s_addr = inet_addr (__host);
    }

  if (setsockopt (sock, SOL_SOCKET, SO_REUSEADDR,
                  (void *) & sock_opt, sizeof (sock_opt)) < 0)
    {
      return -1;
    }

  if (bind (sock, (struct sockaddr*) & name, sizeof (name)) != 0)
    {
      return -1;
    }

  return sock;
}

/**
 * Destroy created socket
 *
 * @param __sock - socket ot destroy
 * @return zero on success, non-zero otherwise
 */
int
sock_destroy (int __sock)
{
  /* Totally */
  shutdown (__sock, 2);

  if (close (__sock) < 0)
    {
      return -1;
    }

  return 0;
}

/**
 * Set non-blocking option to socket
 *
 * @param __sock - socket to operate with
 * @param __val - non-zero to set socket as non-blocking, zero otherwise
 * @return zero on success, non-zero otherwise
 */
int
sock_set_nonblock (int __sock, int __val)
{
  int old_flags = fcntl (__sock, F_GETFL, 0);

  if (old_flags == -1)
    {
      return -1;
    }

  if (__val != 0)
    {
      old_flags |= O_NONBLOCK;
    }
  else
    {
      old_flags &= ~O_NONBLOCK;
    }

  fcntl (__sock, F_SETFL, old_flags);

  return 0;
}

/**
 * Set noinherit option to socket
 *
 * @param __sock - socket to operate with
 * @param __val - non-zero to set socket as noinherit, zero otherwise
 * @return zero on success, non-zero otherwise
 */
int
sock_set_noinherit (int __sock, int __val)
{
  int old_flags = fcntl (__sock, F_GETFD, 0);

  if (old_flags == -1)
    {
      return -1;
    }

  if (__val != 0)
    {
      old_flags |= FD_CLOEXEC;
    }
  else
    {
      old_flags &= ~FD_CLOEXEC;
    }

  fcntl (__sock, F_SETFD, old_flags);

  return 0;
}

/**
 * Formatted answer to socket
 *
 * @param __sock - socket to answer to
 * @param __text - formatted text
 * @return zero on success, non-zero otherwise
 */
int
sock_answer (int __sock, const char* __text, ...)
{
  char print_buf[BUFSIZE];
  int result;

  PACK_ARGS (__text, print_buf, BUFSIZE);

  result = send (__sock, print_buf, strlen (print_buf), MSG_NOSIGNAL);

  if (result < 0)
    {
      if (errno != EWOULDBLOCK)
        {
          return -1;
        }
    }

  return 0;
}

/**
 * Read buffer from socket
 *
 * @param __sock - socket ot read from
 * @oaram __maxlen - maximal length of buffer to read
 * @param __out - pointer to output buffer
 * @return number of received bytes
 */
int
sock_read (int __sock, int __maxlen, char* __out)
{
  int len;

  len = recv (__sock, __out, __maxlen, 0);

  if (len < 0)
    {
      if (errno != EWOULDBLOCK)
        {
          return -1;
        }
    }

  __out[len] = 0;

  return len;
}

/**
 * Create INET socket for client
 *
 * @param __hostname - hostname or IP address of server
 * @param __port - port of server socket
 * @return socket on success, -1 otherwise
 */
int
sock_create_client_inet (const char *__hostname, unsigned int __port)
{
  int sock;
  struct sockaddr_in name;
  struct hostent *host;

  name.sin_family = AF_INET;
  name.sin_port = htons (__port);
  host = gethostbyname (__hostname);

  if (!host)
    {
      return SE_UNKNOWN_HOST;
    }

  name.sin_addr = *(struct in_addr*) host->h_addr;
  sock = socket (PF_INET, SOCK_STREAM, 0);

  if (sock < 0)
    {
      return SE_CANT_CREATE_SOCK;
    }

  if (connect (sock, (struct sockaddr*) & name, sizeof (name)) < 0)
    {
      sock_destroy (sock);
      return SE_CANT_CONNECT;
    }

  sock_set_nonblock (sock, 0);
  return sock;
}

/**
 * Create UNIX socket for server
 *
 * @param __fn - name of file
 * @return socket on success, -1 otherwise
 */
int
sock_create_unix (const char *__fn)
{
  int sock, len;
  struct sockaddr_un local;

  if ((sock = socket (AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
      return SE_CANT_CREATE_SOCK;
    }

  local.sun_family = AF_UNIX;
  strcpy (local.sun_path, __fn);
  unlink (local.sun_path);
  len = strlen (local.sun_path) + sizeof (local.sun_family);

  if (bind (sock, (struct sockaddr *) & local, len) == -1)
    {
      return -1;
    };

  return sock;
}

/**
 * Create UNIX socket for client
 *
 * @return socket on success, -1 otherwise
 */
int
sock_create_client_unix (const char *__fn)
{
  int sock, len;
  struct sockaddr_un remote;

  if ((sock = socket (AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
      return SE_CANT_CREATE_SOCK;
    }

  remote.sun_family = AF_UNIX;
  strcpy (remote.sun_path, __fn);
  len = strlen (remote.sun_path) + sizeof (remote.sun_family);

  if (connect (sock, (struct sockaddr *) & remote, len) == -1)
    {
      return -1;
    };

  return sock;
}
