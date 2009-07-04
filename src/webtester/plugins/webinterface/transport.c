/**
 * WebTester Server - server of on-line testing system
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include <libwebtester/core.h>
#include <libwebtester/network-soup.h>
#include <libwebtester/smartinclude.h>
#include <libwebtester/conf.h>
#include <libwebtester/regexp.h>
#include <libwebtester/assarr.h>
#include <libwebtester/flexval.h>

#include <malloc.h>
#include <stdlib.h>

#include <webtester/belts.h>
#include <webtester/stat.h>

#include "transport.h"

#define MAX_URL_LEN 65536

#define CHECK_ERROR(__proc) \
  if (!HTTP_STATUS_OK (*msg)) \
    { \
      char buf[1024]; \
      http_get_error (msg, buf); \
      core_print (MSG_ERROR, "    Error sending HTTP request for `%s`: %s\n", \
                  __proc,buf); \
      http_message_free (msg); \
      return -1; \
    }

static http_session_t *http_session = NULL;
static BOOL use_ssl = FALSE;
static char proxy[1024] = {0};
static char ssl_ca_file[4096] = {0};

static unsigned long long bsend = 0;
static unsigned long long brecv = 0;

/****
 * General stuff
 */

/**
 * Read data from config file
 */
static void
read_config (void)
{
  flex_value_t *use_ssl_val = 0;
  CONFIG_OPEN_KEY (use_ssl_val, "Server/Plugins/WebInterface/UseSSL");

  if (use_ssl_val && flexval_is_truth (use_ssl_val))
    {
      use_ssl = TRUE;
    }

  CONFIG_PCHAR_KEY (proxy, "Server/Plugins/WebInterface/Proxy");
  CONFIG_PCHAR_KEY (ssl_ca_file, "Server/Plugins/WebInterface/SSL-CA-File");
}

/**
 * Make HTTP connection
 *
 * @return zero on success, non-zero otherwise
 */
static int
make_http_connection (void)
{
  if (strcmp (ssl_ca_file, ""))
    use_ssl = TRUE;

  /* Create synchronizing HTTP session */
  http_session = http_session_new_extended (FALSE, proxy,
                ((use_ssl && strcmp (ssl_ca_file, "")) ? (ssl_ca_file) : (0)));

  if (!http_session)
    {
      return -1;
    }

  return 0;
}

/****
 * URL generating stuff
 */

/**
 * Preparing of IPC URL
 *
 * @param __action - action to all on remote side
 * @param __out - output buffer
 */
static void
prepare_url (const char *__action, char *__out)
{
  static char url_prefix[MAX_URL_LEN] = "";

  if (!strlen (url_prefix))
    {
      /* Prepare general URL */
      char addr[1024] = "", gateway[1024] = "", login[64] = "",
              pass1[64] = "", pass2[64] = "";
      char *proto = ((use_ssl) ? ("https") : ("http"));

      /* Get some config */
      CONFIG_PCHAR_KEY (addr, "Server/Plugins/WebInterface/INET_Addr");
      CONFIG_PCHAR_KEY (gateway, "Server/Plugins/WebInterface/INET_Gateway");
      CONFIG_PCHAR_KEY (login, "Server/Plugins/WebInterface/INET_Login");
      CONFIG_PCHAR_KEY (pass1, "Server/Plugins/WebInterface/INET_Pass1");
      CONFIG_PCHAR_KEY (pass2, "Server/Plugins/WebInterface/INET_Pass2");

      if (!strcmp (addr, ""))
        {
          strcpy (addr, "localhost");
        }

      if (!strcmp (gateway, ""))
        {
          strcpy (gateway, "/gateway/index.php");
        }

      snprintf (url_prefix, BUF_SIZE (url_prefix),
                "%s://%s%s?login=%s&pass1=%s&pass2=%s",
                proto, addr, gateway, login, pass1, pass2);

      memset (login, 0, sizeof (login));
      memset (pass1, 0, sizeof (pass1));
      memset (pass2, 0, sizeof (pass2));
    }

  /* Set general URL and params */
  sprintf (__out, "%s&ipc=%s", url_prefix, __action);
}

/**
 * Get URL for getting task list
 *
 * @param __out - output buffer
 */
static void
get_tasklist_url (char *__out)
{
  prepare_url ("get_task_list", __out);
}

/**
 * Get URL for deleting task
 *
 * @param __out - output buffer
 */
static void
get_deltask_url (char *__out)
{
  prepare_url ("delete_task", __out);
}

/**
 * Get URL for restoring task
 *
 * @param __out - output buffer
 */
static void
get_restoretask_url (char *__out)
{
  prepare_url ("restore_task", __out);
}

/**
 * Get URL for getting detailed task information
 *
 * @param __out - output buffer
 */
static void
get_gettask_url (char *__out)
{
  prepare_url ("get_task", __out);
}

/**
 * Get URL for sending info about tested task
 *
 * @param __out - output buffer
 */
static void
get_putsolution_url (char *__out)
{
  prepare_url ("put_solution", __out);
}

/**
 * Get URL for sending info about tested task
 *
 * @param __out - output buffer
 */
static void
get_resetstatus_url (char *__out)
{
  prepare_url ("reset_status", __out);
}

/****
 * Messaging stuff
 */

/**
 * Send simple message with URL
 *
 * @param __url - url to send to
 * @return sent message
 */
static http_message_t*
send_message (const char *__url)
{
  char dummy[1024];

  http_message_t *msg = http_message_prepare ("GET", __url);

  if (!msg)
    {
      return 0;
    }

  soup_message_set_flags (msg, SOUP_MESSAGE_NO_REDIRECT);
  http_session_send_message (http_session, msg);

  bsend += strlen (__url);
  brecv += HTTP_RESPONSE_LENGTH (msg);

  /*
   * TODO: Use pchar-ed data because flexval stuff
   *       works only with long-typed numbers
   */

  snprintf (dummy, BUF_SIZE (dummy), "%lld", bsend);
  wt_stat_set_string ("Plugins.WebInterface.BytesSend", dummy);

  snprintf (dummy, BUF_SIZE (dummy), "%lld", brecv);
  wt_stat_set_string ("Plugins.WebInterface.BytesRecv", dummy);

  return msg;
}

/**
 * Send message with specified solution and library ID
 *
 * @param __prefix - url prefix
 * @param __sid - solution ID
 * @param __lid - library ID
 * @return sent message
 */
static http_message_t*
send_task_specified_message (const char *__prefix, long __sid, int __lid)
{
  char url[MAX_URL_LEN];
  snprintf (url, BUF_SIZE (url), "%s&id=%ld&lid=%d", __prefix, __sid, __lid);
  return send_message (url);
}

/**
 * Send message for getting task list
 *
 * @return sent message
 */
static http_message_t*
send_gettasklist_message (void)
{
  char url[MAX_URL_LEN];

  /* Get url */
  get_tasklist_url (url);
  return send_message (url);
}

/**
 * Send message for deleting task
 *
 * @param __sid - solution ID
 * @param __lid - library ID
 * @return sent message
 */
static http_message_t*
send_deltask_message (long __sid, int __lid)
{
  char url[MAX_URL_LEN];
  get_deltask_url (url);
  return send_task_specified_message (url, __sid, __lid);
}

/**
 * Send message for restoring task
 *
 * @param __sid - solution ID
 * @param __lid - library ID
 * @return sent message
 */
static http_message_t*
send_restoretask_message (long __sid, int __lid)
{
  char url[MAX_URL_LEN];
  get_restoretask_url (url);
  return send_task_specified_message (url, __sid, __lid);
}

/**
 * Send message for getting task
 *
 * @param __sid - solution ID
 * @param __lid - library ID
 * @return sent message
 */
static http_message_t*
send_gettask_message (long __sid, int __lid)
{
  char url[MAX_URL_LEN];
  get_gettask_url (url);
  return send_task_specified_message (url, __sid, __lid);
}

/**
 * Send message for putting tested task
 *
 * @param __sid - solution ID
 * @param __lid - library ID
 * @return sent message
 */
static http_message_t*
send_putsolution_message (long __sid, int __lid, char *__add)
{
  char url[MAX_URL_LEN];
  get_putsolution_url (url);
  strcat (url, "&");
  strcat (url, __add);
  return send_task_specified_message (url, __sid, __lid);
}

/**
 * Send message for restoring statuses
 *
 * @param __sid - solution ID
 * @param __lid - library ID
 * @return sent message
 */
static http_message_t*
send_resetstatus_message (void)
{
  char url[MAX_URL_LEN];
  get_resetstatus_url (url);
  return send_message (url);
}

/****
 * User's backend
 */

/**
 * Initialize WebIFACE transport stuff
 *
 * @return zero on success, non-zero otherwise
 */
int
webiface_transport_init (void)
{
  read_config ();

  if (make_http_connection ())
    {
      return -1;
    }

  return 0;
}

/**
 * Uninitialize WebIFACE transport stuff
 */
void
webiface_transport_done (void)
{
  if (http_session)
    {
      http_session_free (http_session);
    }
}

/**
 * Delete task from testing queue
 *
 * @param __self - task to be deleted
 * @return sero on success, non-zero otherwise
 */
int
webiface_delete_task (wt_task_t *__self)
{
  http_message_t *msg;
  msg = send_deltask_message (__self->sid, __self->lid);

  if (!msg)
    {
      return -1;
    }

  CHECK_ERROR ("delete_task");
  http_message_free (msg);
  return 0;
}

/**
 * Restore task in testing queue
 *
 * @param __self - task to be restored
 * @return sero on success, non-zero otherwise
 */
int
webiface_restore_task (wt_task_t *__self)
{
  http_message_t *msg;
  msg = send_restoretask_message (__self->sid, __self->lid);

  if (!msg)
    {
      return -1;
    }

  CHECK_ERROR ("restore_task");
  http_message_free (msg);
  return 0;
}

/**
 * Get detailed task information
 *
 * @param __self - task to be detalized
 * @return sero on success, non-zero otherwise
 */
int
webiface_get_task (wt_task_t *__self)
{
  http_message_t *msg;

  if (TASK_TEST_FLAG (*__self, TF_UNPACKED))
    {
      return 0;
    }

  msg = send_gettask_message (__self->sid, __self->lid);
  if (!msg)
    {
      return -1;
    }

  CHECK_ERROR ("get_task");

  if (HTTP_RESPONSE_LENGTH (msg) <= 0)
    {
      http_message_free (msg);
      return -1;
    }

  assarr_unpack ((char*) HTTP_RESPONSE_BODY (msg), __self->input_params);
  http_message_free (msg);

  TASK_SET_FLAG (*__self, TF_UNPACKED);

  return 0;
}

/**
 * Return tested task to WebIFACE
 *
 * @param __self - task to be returned
 * @return sero on success, non-zero otherwise
 */
int
webiface_put_solution (wt_task_t *__self)
{
  char add[MAX_URL_LEN], *key, *value, dummy[MAX_URL_LEN];
  http_message_t *msg;

  /* Collect additional params */
  strcpy (add, "");

  ASSARR_FOREACH_DO (__self->output_params, key, value);
  if (strcmp (add, ""))
    {
      strcat (add, "&");
    }
  strcat (add, key);
  strcat (add, "=");
  urlencode (value, dummy);
  strcat (add, dummy);
  ASSARR_FOREACH_DONE;

  /* Send message */
  msg = send_putsolution_message (__self->sid, __self->lid, add);
  if (!msg)
    {
      return -1;
    }

  CHECK_ERROR ("put_solution");
  http_message_free (msg);
  return 0;
}

/**
 * Reset statuf of half-tested tasks
 *
 * @return sero on success, non-zero otherwise
 */
int
webiface_reset_status (void)
{
  http_message_t *msg;
  msg = send_resetstatus_message ();

  if (!msg)
    {
      return -1;
    }

  CHECK_ERROR ("reset_status");
  http_message_free (msg);
  return 0;
}

/**
 * Recieveng list of untested problems
 *
 * @param __tasklist - list to store tasks
 * @param __queue_size - max queue size
 * @return sero on success, non-zero otherwise
 */
int
webiface_get_task_list (dynastruc_t *__tasklist, int __queue_size)
{
  wt_task_t *ptr;
  http_message_t *msg;

  /* Check for posibility of getting tasks */
  if (dyna_length (__tasklist) >= __queue_size)
    {
      return -1;
    }

  msg = send_gettasklist_message ();
  if (!HTTP_STATUS_OK (*msg)) /* Error while sending message */
    {
      char buf[1024];
      http_get_error (msg, buf);
      core_print (MSG_ERROR,
                  "    Error sending HTTP request for tasklist: %s\n", buf);
    }
  else
    {
      char body[65536];
      if (HTTP_RESPONSE_LENGTH (msg) > 0)
        {
          strncpy (body, HTTP_RESPONSE_BODY (msg), 65535);
          body[HTTP_RESPONSE_LENGTH (msg)] = 0;
          if (!preg_match ("/^([0-9]+@[0-9]+\n)*([0-9]+@[0-9]+)?$/s", body))
            {
              /* Check for valid of retururned buffer */
              core_print (MSG_ERROR, "    Got invalid tasklist throug IPC.\n");
            }
          else
            {
              char token[1024], sid_pchar[64];
              int i = 0, len = strlen (body), tlen, lid, taked = 0;
              int need = __queue_size - dyna_length (__tasklist);
              long sid;

              /* Getting of tasklist */
              core_print (MSG_INFO, "    Recieving max of %d tasks...\n", need);

              /* Parsing body */
              while (i < len)
                {
                  if (taked >= need) break;
                  /* Skip spaces */
                  if ((body[i] < '0' || body[i] > '9') && body[i] != '@')
                    {
                      while (i < len && (body[i] < '0' || body[i] > '9')
                              && body[i] != '@')
                        {
                          i++;
                        }
                    }

                  /* Collect token */
                  tlen = 0;
                  lid = -1;
                  strcpy (sid_pchar, "");
                  while (i < len && ((body[i] >= '0' && body[i] <= '9') ||
                          body[i] == '@'))
                    {
                      if (body[i] == '@') /* Solution id is taked */
                        {
                          token[tlen] = 0;
                          tlen = 0;
                          strcpy (sid_pchar, token);
                        }
                      else
                        {
                          token[tlen++] = body[i];
                        }
                      i++;
                    }

                  if (!strcmp (sid_pchar, "") || !tlen)
                    {
                      break; /* Maybe spacing characters at the end of token */
                    }

                  token[tlen] = 0;
                  lid = atoi (token);
                  sid = atol (sid_pchar);

                  /* Append task to queue */
                  ptr = wt_spawn_new_task (sid, lid);
                  dyna_search_reset (__tasklist);
                  if (dyna_search (__tasklist, ptr, 0,
                                   wt_task_search_comparator))
                    {
                      core_print (MSG_INFO, "        Ignore recieved task"
                                            " %ld@%d (already in queue)\n",
                                  sid, lid);
                      wt_task_free (ptr);
                    }
                  else if (wt_task_in_belts (ptr))
                    {
                      core_print (MSG_INFO, "        Ignore recieved task "
                                            "%ld@%d (already in belts)\n",
                                  sid, lid);
                      wt_task_free (ptr);
                    }
                  else
                    {
                      core_print (MSG_INFO, "        Recieved task %ld@%d\n",
                                  sid, lid);
                      dyna_append (__tasklist, ptr, 0);

                      /* Mark task as taked */
                      webiface_delete_task (ptr);
                      taked++;
                    }
                }
              core_print (MSG_INFO, "    Recieving tasklist finished\n");
            }
        }
    }

  http_message_free (msg);

  return 0;
}

/**
 * Prepare URL to send
 *
 * @param __self - string to be prepared
 * @param __out - output buffer
 */
void
webiface_prepare_url (const char *__self, char *__out)
{
  prepare_url (__self, __out);
}

/**
 * Send simple message with URL
 *
 * @param __url - URL to send message to
 * @retur nsent message
 */
http_message_t*
webiface_send_message (const char *__url)
{
  return send_message (__url);
}

/**
 * Get number of bytes send
 *
 * @return number of bytes send
 */
DWORD
webiface_bytes_send (void)
{
  return bsend;
}

/**
 * Get number of bytes received
 *
 * @return number of bytes received
 */
DWORD
webiface_bytes_recv (void)
{
  return brecv;
}
