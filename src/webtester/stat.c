/*
 *
 * ================================================================================
 *  stat.h - part of the WebTester Server Server
 * ================================================================================
 *
 *  Statistics stuff.
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include "autoinc.h"
#include "stat.h"
#include "ipc.h"

#include <malloc.h>

#include <libwebtester/ipc.h>
#include <libwebtester/hook.h>
#include <libwebtester/assarr.h>

////////
// type defenitnions

typedef struct {
  ipc_client_t *ipc_client;

  BOOL registered;
  char uid[128];

  assarr_t *vars;
} stat_client_t;

////
//

static stat_client_t clients[IPC_MAX_CLIENT_COUNT];
static assarr_t *vars = NULL;
static assarr_t *desc = NULL;

static BOOL initialized=FALSE;

////////
//

static void
dump_arr_to_buf                    (assarr_t *__self, char *__buf)
{
  flex_value_t *val;
  char *key;
  char dumped[65536], dummy[1024];
  
  strcpy (__buf, "");

  ASSARR_FOREACH_DO (__self, key, val);
    flexval_serialize (val, dumped);
    strcat (__buf, key);
    strcat (__buf, ";");
    sprintf (dummy, "%ld", (long)strlen (dumped));
    strcat (__buf, dummy);
    strcat (__buf, ";");
    strcat (__buf, dumped);
    strcat (__buf, ";");
  ASSARR_FOREACH_DONE;
}

static void
var_deleter                        (void *__self)
{
  if (!__self) return;
  flexval_free (__self);
  free (__self);
}

static void
review_clients                     (void)
{
  int i;

  for (i=0; i<IPC_MAX_CLIENT_COUNT; i++)
    {
      // Mistmatch between stored and real client's UID
      if (strcmp (clients[i].ipc_client->uid, clients[i].uid))
        {
          // Unset unwanted data
          if (clients[i].vars)
            assarr_unset_all (clients[i].vars, var_deleter);
          clients[i].registered=FALSE;

          // Update data
          strcpy (clients[i].ipc_client->uid, clients[i].uid);
        }
    }
}

static void
build_stat_changes                 (stat_client_t *__self, assarr_t *__vars_diff)
{
  if (!__self) return;

  flex_value_t *val, *cval;
  char *key;

  ASSARR_FOREACH_DO (vars, key, val);
    cval=assarr_get_value (__self->vars, key);
    if (!cval || flexval_cmp (val, cval))
      assarr_set_value (__vars_diff, key, val);
  ASSARR_FOREACH_DONE;
}

static void
send_stat_changes                  (stat_client_t *__self, assarr_t *__vars)
{
  char stat[65536], vars[65536];

  if (__self->ipc_client->access<7) // But why??
    return;
  
  dump_arr_to_buf (__vars, vars);
  sprintf (stat, "vars;%ld;%s;", (long)strlen (vars), vars);

  if (strcmp (vars, ""))
    sock_answer (__self->ipc_client->sock, "+OK %s\n", stat);
}

static void
update_client_stat_cache           (stat_client_t *__self, assarr_t *__vars)
{
  flex_value_t *val, *cval;
  char *key;

  if (!__self->vars)
    __self->vars=assarr_create ();

  ASSARR_FOREACH_DO (__vars, key, val);
    cval=assarr_get_value (__self->vars, key);
    if (!cval)
      {
        cval=malloc (sizeof (flex_value_t));
        flexval_create (cval);
        assarr_set_value (__self->vars, key, cval);
      }
    flexval_copy (val, cval);
  ASSARR_FOREACH_DONE;
}

static int
stat_changed_callback              (void)
{
  int i;

  assarr_t *vars_diff=NULL;

  review_clients ();

  for (i=0; i<IPC_MAX_CLIENT_COUNT; i++)
    {
      if (clients[i].registered)
        {
          if (!vars_diff)
            vars_diff=assarr_create ();
          assarr_unset_all (vars_diff, 0);
          build_stat_changes (&clients[i], vars_diff);
          send_stat_changes (&clients[i], vars_diff);
          update_client_stat_cache (&clients[i], vars_diff);
        }
    }

  if (vars_diff)
    assarr_destroy (vars_diff, 0);

  return 0;
}

static flex_value_t*
get_var                            (char *__self)
{
  flex_value_t *fv=assarr_get_value (vars, __self);
  if (fv)
    return fv;
  fv=malloc (sizeof (flex_value_t));
  flexval_create (fv);
  assarr_set_value (vars, __self, fv);
  return fv;
}

static void
dump_stat_to_buf                   (char *__buf)
{
  char svars[65536];
  dump_arr_to_buf (vars, svars);
  sprintf (__buf, "vars;%ld;%s;", (long)strlen (svars), svars);
}

static void
send_unpacked_stat                 (stat_client_t *__self)
{
  char *key, buf[4096], *dummy;
  flex_value_t *val;

  sock_answer (__self->ipc_client->sock, "+OK ");

  ASSARR_FOREACH_DO (vars, key, val)
    flexval_serialize (val, buf);
    dummy=assarr_get_value (desc, key);
    sock_answer (__self->ipc_client->sock, "%s %s\n", (dummy)?(dummy):(key), buf);
  ASSARR_FOREACH_DONE
}

static void     // Send full stat to client
send_stat_to_client                (stat_client_t *__self, BOOL __packed)
{
  if (__packed)
    {
      char stat[65536];

      if (__self->ipc_client->access<7) // But why??
        return;
  
      dump_stat_to_buf (stat);
      sock_answer (__self->ipc_client->sock, "+OK %s\n", stat);
  } else {
    send_unpacked_stat (__self);
  }
}

////
//

static stat_client_t*
get_client_by_ipc_info             (ipc_client_t *__info)
{
  review_clients ();
  return &clients[__info->id];
}

static int      // Handler of IPC command `stat`
ipc_stat                           (int __argc, char **__argv)
{
  stat_client_t *client=get_client_by_ipc_info (ipc_get_current_client ());

  IPC_ADMIN_REQUIRED

  if (__argc==2)
    {
      if (!strcmp (__argv[1], "help"))
        {
          IPC_PROC_ANSWER ("+OK Usage: stat [register|unregister]\n  `stat` prints the statistics\n`stat register` regsiters you as client to receive all "
            "changes of statistic\n"
            "`stat unregister` makes opposite action\n");
        } else
      if (!strcmp (__argv[1], "register"))
        {
          if (!client->registered)
            {
              client->registered=TRUE;
              IPC_PROC_ANSWER ("+OK\n");
            } else
              IPC_PROC_ANSWER ("-ERR Ypu have been already registered as STAT client\n");
        } else
      if (!strcmp (__argv[1], "unregister"))
        {
          if (client->registered)
            {
              client->registered=FALSE;
              IPC_PROC_ANSWER ("+OK\n");
            } else
              IPC_PROC_ANSWER ("-ERR You isn't STAT client\n");
        } else
      if (!strcmp (__argv[1], "packed"))
        {
          send_stat_to_client (client, TRUE);
        } else
          goto __usage_;
    } else
  if (__argc==1)
    {
      send_stat_to_client (client, FALSE);
    } else
      goto __usage_;

  return 0;
__usage_:
  IPC_PROC_ANSWER ("-ERR Type `stat help` for help\n");
  return 0;
}

////
//

static void
clients_init                       (void)
{
  int i;
  if (!clients)
    return ;

  memset (clients, 0, sizeof (clients));
  for (i=0; i<IPC_MAX_CLIENT_COUNT; i++)
    {
      clients[i].ipc_client=ipc_get_client_by_id (i);
    }
}

static void
clients_done                       (void)
{
  int i;
  memset (clients, 0, sizeof (clients));
  for (i=0; i<IPC_MAX_CLIENT_COUNT; i++)
    {
      if (clients[i].vars)
      assarr_destroy (clients[i].vars, var_deleter);
    }
}

////////
// User's backend

int
wt_stat_init                       (void)
{
  if (!wt_ipc_supported ())
    return -1;

  clients_init ();

  vars=assarr_create ();
  desc=assarr_create ();

  ipc_proc_register ("stat", ipc_stat);

//  hook_register ("Stat.Changed", stat_changed_callback, 0, HOOK_PRIORITY_NORMAL);

  initialized=TRUE;

  return 0;
}

void
wt_stat_done                       (void)
{
  if (!initialized)
    return;

  initialized=FALSE;
  clients_done ();

  assarr_destroy (vars, var_deleter);
  assarr_destroy (desc, assarr_deleter_free_ref_data);

//  hook_unregister ("Stat.Changed", stat_changed_callback, HOOK_PRIORITY_NORMAL);
}

////
//

void
wt_stat_set_int                    (char *__var, long __val)
{
  if (!initialized) return;
  flex_value_t *fv=get_var (__var);
  flexval_set_int (fv, __val);
  stat_changed_callback ();
}

void
wt_stat_set_float                  (char *__var, double __val)
{
  if (!initialized) return;
  flex_value_t *fv=get_var (__var);
  flexval_set_float (fv, __val);
  stat_changed_callback ();
}

void
wt_stat_set_string                 (char *__var, char *__val)
{
  if (!initialized) return;
  flex_value_t *fv=get_var (__var);
  flexval_set_string (fv, __val);
  stat_changed_callback ();
}

void
wt_stat_set_array                  (char *__var, flex_value_t **__val)
{
  if (!initialized) return;
  flex_value_t *fv=get_var (__var);
  flexval_set_array (fv, __val);
  stat_changed_callback ();
}

void
wt_stat_set_desc                   (char *__var, char *__desc)
{
  if (!__var)
    return;

  assarr_unset_value (desc, __var, assarr_deleter_free_ref_data);

  if (!__desc)
    return;

  assarr_set_value (desc, __var, strdup (__desc));
}
