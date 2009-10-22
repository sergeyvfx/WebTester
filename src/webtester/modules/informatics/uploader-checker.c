/**
 * WebTester Server - server of on-line testing system
 *
 * Checker uploading stuff
 *
 * Copyright 2008 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include <libwebtester/assarr.h>
#include <libwebtester/regexp.h>
#include <libwebtester/fs.h>

#include <librun/run.h>

#include <webtester/transport.h>

#include "informatics.h"
#include "macros.h"

#include <unistd.h>
#include <sys/stat.h>
#include <malloc.h>

static BOOL active = TRUE;

#define CHECK_ACTIVE() \
  if (!active) { \
    core_print (MSG_INFO, "    Checker uploadging interrupted.\n"); \
    return FALSE; \
  }

#define CHECK_ACTIVE_VOID() \
  if (!active) { \
    core_print (MSG_INFO, "    Checker uploadging interrupted.\n"); \
    return; \
  }

static char *required_params[] = {
  "ID",
  "COMPILERID",
  "SRC",
  0
};

/**
 * Send IPC command to get detailed checker parameters
 *
 * @param __params - params to fill
 * @return TRUE on success, FALSE otherwise
 */
static BOOL
send_ipc_request (assarr_t *__params)
{
  static char url[4096];
  static BOOL initialized = FALSE;

  http_message_t *msg;

  if (!initialized)
    {
      wt_transport_prepare_url ("get_checker", url);
      initialized = TRUE;
    }

  msg = wt_transport_send_message (url);

  if (!msg)
    {
      return FALSE;
    }

  if (!HTTP_STATUS_OK (*msg))
    {
      char err[1024];
      http_get_error (msg, err);
      core_print (MSG_ERROR, "    Informatics: Error sending HTTP request "
                             "for checker's list: %s\n", err);

      http_message_free (msg);
      return FALSE;
    }

  if (HTTP_RESPONSE_LENGTH (msg) > 0)
    {
      assarr_unpack ((char*) HTTP_RESPONSE_BODY (msg), __params);
    }

  http_message_free (msg);

  return TRUE;
}

/**
 * Get full directory where checkers are stored
 *
 * @param __out - output buffer
 */
static void
full_checkers_dir (char *__out)
{
  char data_dir[4096], checkers_storage[4096];
  INF_PCHAR_KEY (data_dir, "DataDir");
  INF_PCHAR_KEY (checkers_storage, "Checker/StorageDir")
  sprintf (__out, "%s/%s", data_dir, checkers_storage);
}

/**
 * Save checker
 *
 * @param __params - checker's parameters
 * @return TRUE on success, FALSE otherwise
 */
static BOOL
save_checker (assarr_t *__params)
{
  char *compiler_id = assarr_get_value (__params, "COMPILERID");
  char *ext = COMPILER_SAFE_PCHAR_KEY (compiler_id, "Extension",
                                       INFORMATICS_SRCEXT);
  char *src;
  char dir[4096], fn[4096];

  CHECK_ACTIVE ();

  full_checkers_dir (dir);

  /* Make checkers' storage root */
  fmkdir (dir, 00775);

  snprintf (fn, BUF_SIZE (fn), "%s/%s%s", dir,
            (char*) assarr_get_value (__params, "ID"), ext);

  src = assarr_get_value (__params, "SRC");
  if (fwritebuf (fn, src))
    {
      return FALSE;
    }

  chmod (fn, 00664);

  return TRUE;
}

/**
 * Build command to compile checker
 *
 * @param __params - cheker's parameters
 * @param __out - output buffer
 */
static void
build_compilation_cmd (assarr_t *__params, char *__out)
{
  char *compiler_id = assarr_get_value (__params, "COMPILERID");
  char *ext = COMPILER_SAFE_PCHAR_KEY (compiler_id, "Extension",
                                       INFORMATICS_SRCEXT);
  char *cmd = COMPILER_SAFE_PCHAR_KEY (compiler_id, "Command", "");
  char *id = assarr_get_value (__params, "ID");

  char source[4096];
  char flags[4096] = "", flags_path[1024];

  snprintf (source, BUF_SIZE (source), "%s%s", id, ext);

  strcpy (__out, cmd);

  snprintf (flags_path, BUF_SIZE (flags_path),
            "Checker/CompilerFlags/%s", compiler_id);
  INF_PCHAR_KEY (flags, flags_path);

  REPLACE_VAR (__out, "flags", flags);
  REPLACE_VAR (__out, "source", source);
  REPLACE_VAR (__out, "output", id);
}

/**
 * Execute compiler
 *
 * @param __params - cheker's parameters
 * @param __cmd - command to execute
 * @param __err - buffer to store errors from compiler
 */
static BOOL
exec_compiler (assarr_t *__params, const char *__cmd, char *__err)
{
  char *compiler_id = assarr_get_value (__params, "COMPILERID");
  char dir[4096];

  DWORD compiler_ml;
  DWORD compiler_tl;
  run_process_info_t *proc;
  BOOL result = TRUE;

  full_checkers_dir (dir);

  /* Get compiler's limits */
  /* Memory limit */
  compiler_ml = COMPILER_SAFE_INT_KEY (compiler_id, "Limits/RSS",
                                       INFORMATICS_COMPILER_RSS_LIMIT);
  /* Time limit */
  compiler_tl = COMPILER_SAFE_FLOAT_KEY (compiler_id, "Limits/Time",
                                 INFORMATICS_COMPILER_TIME_LIMIT) * USEC_COUNT;

  /*  Create process */
  INF_DEBUG_LOG ("checker-uploader: Executing compiler (cmd: %s)\n", __cmd);
  proc = run_create_process (__cmd, dir, compiler_ml, compiler_tl);
  run_execute_process (proc); /* Execute process and.. */
  run_pwait (proc); /* ..wait finishing of process */
  INF_DEBUG_LOG ("checker-uploader: Finish executing compiler\n");

  if (RUN_PROC_EXEC_ERROR (*proc))
    {
      INF_DEBUG_LOG ("checker-uploader: Fatal error executing compiler: %s\n",
                     RUN_PROC_ERROR_DESC (*proc));
      sprintf (__err, "Fatal error executing compiler: %s",
               RUN_PROC_ERROR_DESC (*proc));
      run_free_process (proc);
      return FALSE;
    }

  /* Set output buffer from pipe */
  if (RUN_PROC_PIPEBUF (*proc))
    {
      assarr_set_value (__params, "COMPILER_MESSAGES",
                        strdup (RUN_PROC_PIPEBUF (*proc)));
    }

  /* Nonzero-coded exit - compilation error */
  if (PROCESS_RUNTIME_ERROR (*proc))
    {
      result = FALSE;
    }

  run_free_process (proc);

  return result;
}

/**
 * Compile checker
 *
 * @param __params - checker's parameters
 * @param __err - buffer to store error description
 */
static BOOL
compile (assarr_t *__params, char *__err)
{
  char dir[4096], cmd[4096], fn[4096];
  full_checkers_dir (dir);
  build_compilation_cmd (__params, cmd);

  CHECK_ACTIVE ();

  snprintf (fn, BUF_SIZE (fn), "%s/%s", dir,
            (char*) assarr_get_value (__params, "ID"));

  unlink (fn);

  strcpy (__err, "");

  if (!exec_compiler (__params, cmd, __err))
    {
      strcpy (__err, assarr_get_value (__params, "COMPILER_MESSAGES"));
      return FALSE;
    }

  if (!fexists (fn))
    {
      char *msg = assarr_get_value (__params, "COMPILER_MESSAGES");
      if (!msg)
        {
          strcpy (__err, "Compilation error");
        }
      else
        {
          sprintf (__err, "Compilation error: %s", msg);
        }
      return FALSE;
    }

  chmod (fn, 00775);

  return TRUE;
}

/**
 * Send checjer uploading information to WebIFACE
 *
 * @param __params - checker's parameters
 * @param __err - error descrption
 * @param __desc - error description
 */
static void
put_checker (assarr_t *__params, const char *__err, const char *__desc)
{
  static char url_prefix[4096];
  static BOOL initialized = FALSE;

  char url[4096], desc[65536];

  http_message_t *msg;

  CHECK_ACTIVE_VOID ();

  if (!__desc) __desc = "";

  if (!strcmp (__err, "OK"))
    {
      INF_INFO ("Uploaded checker %s\n",
                (char*) assarr_get_value (__params, "ID"));
    }
  else
    {
      INF_ERROR ("Error uploading checker %s. ERR: %s.\n",
                 (char*) assarr_get_value (__params, "ID"), __err);
    }

  if (!initialized)
    {
      wt_transport_prepare_url ("put_checker", url_prefix);
      initialized = TRUE;
    }

  urlencode (__desc, desc);

  snprintf (url, BUF_SIZE (url), "%s&id=%s&err=%s&desc=%s", url_prefix,
            (char*) assarr_get_value (__params, "ID"), __err, desc);

  msg = wt_transport_send_message (url);

  if (!msg)
    {
      return;
    }

  if (!HTTP_STATUS_OK (*msg))
    {
      core_print (MSG_ERROR, "    Informatics: Error sending HTTP request "
                             "for return checker's uploading result: %d\n",
                  HTTP_STATUS (*msg));
      http_message_free (msg);
    }

  http_message_free (msg);
}

/**
 * Check required parameters
 *
 * @param __self - parameters to check
 * @return TRUE on success, FALSE otherwise
 */
static BOOL
check_params (assarr_t *__self)
{
  int i = 0;
  while (required_params[i])
    {
      if (!assarr_isset (__self, required_params[i]))
        {
          return FALSE;
        }
      i++;
    }
  return TRUE;
}

/**
 * Upload checker callback
 *
 * @return zero on success, non-zero otherwise
 */
int
Informatics_UploadChecker (void *__unused)
{
  assarr_t *params = assarr_create ();
  if (active && send_ipc_request (params))
    {
      if (assarr_isset (params, "ID"))
        {
          INF_DEBUG_LOG ("checker-uploader: Start uploading\n");
          if (check_params (params))
            {
              char err[65536];
              if (!save_checker (params))
                {
                  INF_DEBUG_LOG ("checker-uploader: Error saving source\n");
                  put_checker (params, "ERR", "Unable to save source");
                }
              else
                {
                  if (!compile (params, err))
                    {
                      put_checker (params, "CE", err);
                    }
                  else
                    {
                      put_checker (params, "OK", "");
                    }
                }
            }
          else
            {
              put_checker (params, "ERR", "Required parameter is undefined");
            }
        }
    }
  assarr_destroy (params, assarr_deleter_free_ref_data);
  return 0;
}

/**
 * Stop checker's uploading
 *
 * @return zero on success, non-zero otherwise
 */
int
Informatics_StopCheckerUploading (void *__unused, void *__call_unused)
{
  active = FALSE;
  return 0;
}
