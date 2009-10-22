/**
 * WebTester Server - server of on-line testing system
 *
 * Task uploading stuff
 *
 * Copyright 2008 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include <librun/run.h>

#include <libwebtester/assarr.h>
#include <libwebtester/conf.h>
#include <libwebtester/fs.h>
#include <libwebtester/hive.h>
#include <libwebtester/regexp.h>
#include <libwebtester/network-smb.h>
#include <libwebtester/strlib.h>
#include <libwebtester/recode.h>

#include <libwebtester/mutex.h>

#include <webtester/transport.h>

#include "informatics.h"
#include "macros.h"

#include <memory.h>
#include <unistd.h>
#include <malloc.h>
#include <stdlib.h>

#include <glib.h>

static BOOL active = TRUE;

#define CHECK_ACTIVE() \
  if (!active) { \
    INF_INFO ("Problem uploadging interrupted.\n"); \
    unlinkdir (uploading_tmp_dir); \
    return FALSE; \
  }

#define CHECK_ACTIVE_VOID() \
  if (!active) { \
    INF_INFO ("Problem uploadging interrupted.\n"); \
    return; \
  }

#define FAIL(__desc) \
  { \
    strcpy (__err_desc, __desc); \
    return FALSE; \
  }

#define CHECK_PARAM(__var) \
  if (!hive_open_key (hive_params, __var)) \
    { \
      strcpy (__err_desc, "Undefined required key " __var \
                          " in checker's config file"); \
      return FALSE; \
    }

#define BUFFER_LENGTH 65536

static BOOL smaba_initialized = FALSE;

static char *required_params[] = {
  "ID",
  0
};

static GThread *thread = 0;
static mutex_t mutex = 0;

/**
 * Send IPC command to WebIFACE to get problem's parameters
 *
 * @param __param - known parameters
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
      wt_transport_prepare_url ("get_problem", url);
      strcat (url, "&lid=0");
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
      INF_ERROR ("Error sending HTTP request for problems's list: %s\n", err);

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

/****
 * SAMBA's stuff
 */

/**
 * Check should archives be uploaded from SAMBA server
 *
 * @return non-zero if archives should be uploaded from SAMBA, zero otherwise
 */
static BOOL
upload_from_samba (void)
{
  static int result = -1;

  if (result == -1)
    {
      char server[1024];
      INF_PCHAR_KEY (server, "ProblemUploader/SMB-Server");

      if (strcmp (server, ""))
        {
          result = TRUE;
        }
      else
        {
          result = FALSE;
        }
    }

  return result;
}

/**
 * Initialize SAMBA stuff
 */
static void
local_samba_init (void)
{
  char server[1024], share[1024], workgroup[1024], login[1024], password[1024];

  if (smaba_initialized) return;

  INF_PCHAR_KEY (server,    "ProblemUploader/SMB-Server");
  INF_PCHAR_KEY (share,     "ProblemUploader/SMB-Share");
  INF_PCHAR_KEY (workgroup, "ProblemUploader/SMB-Workgroup");
  INF_PCHAR_KEY (login,     "ProblemUploader/SMB-Login");
  INF_PCHAR_KEY (password,  "ProblemUploader/SMB-Password");

  samba_push_auth_data (server, share, workgroup, login, password);

  memset (workgroup, 0, sizeof (workgroup));
  memset (login, 0, sizeof (login));
  memset (password, 0, sizeof (password));

  smaba_initialized = TRUE;
}

/****
 * Main uploading stuff
 */

/**
 * Create termporary uploading dir
 *
 * @param __id - ID of uploading problem
 * @param __full - buffer to store full temporary directory
 */
static void
create_temporary_dir (const char *__id, char *__full)
{
  static char CORE_temporary_dir[4096] = {0}, informatics_tmp[4096];

  if (!CORE_temporary_dir[0])
    CONFIG_PCHAR_KEY (CORE_temporary_dir, "CORE/TemporaryDir");

  fmkdir (CORE_temporary_dir, 00775);

  snprintf (informatics_tmp, BUF_SIZE (informatics_tmp),
            "%s/Informatics", CORE_temporary_dir);
  sprintf (__full, "%s/uploading", informatics_tmp);

  /* This directory may be not-empty because of */
  /* incorrect finishing of previous WebTester instance */
  unlinkdir (__full);

  fmkdir (informatics_tmp, 00775);
  fmkdir (__full, 00770);
}

/**
 * Upload archive from remote share to local path using SAMBA
 *
 * @param __fn - name of archive to upload
 * @param __local_path - local path to store archive
 * @return TRUE in success, FALSE otherwise
 */
static BOOL
upload_archive_through_samba (const char *__fn, const char *__local_path)
{
  static char url_prefix[4096] = {0};
  char server_fn[4096], local_fn[4096];
  int fd;
  FILE *stream;
  char buf[BUFFER_LENGTH];
  long len;

  if (!url_prefix[0])
    {
      char server[1024], share[1024], problems_root[4096];
      INF_PCHAR_KEY (server, "ProblemUploader/SMB-Server");
      INF_PCHAR_KEY (share, "ProblemUploader/SMB-Share");
      INF_PCHAR_KEY (problems_root, "ProblemUploader/ServerProblemsRoot");

      snprintf (url_prefix, BUF_SIZE (url_prefix),
                "smb://%s/%s/%s", server, share, problems_root);
    }

  /* Initialize SAMBA stuff */
  local_samba_init ();

  snprintf (server_fn, BUF_SIZE (server_fn), "%s/%s", url_prefix, __fn);
  snprintf (local_fn, BUF_SIZE (local_fn), "%s/%s", __local_path, __fn);

  /* Open file descriptor */
  fd = samba_fopen (server_fn, O_RDONLY, 0);

  if (fd < 0)
    {
      return FALSE;
    }

  stream = fopen (local_fn, "wb");
  if (!stream)
    {
      samba_fclose (fd);
      return FALSE;
    }

  /* Copy-pasting data from remote SAMBA's share to local file */
  do
    {
      len = samba_fread (fd, buf, sizeof (buf));
      if (len > 0)
        {
          fwrite (buf, sizeof (char), len, stream);
        }
    }
  while (len > 0);

  /* Close file descriptor */
  samba_fclose (fd);
  fclose (stream);

  samba_unlink (server_fn);

  return TRUE;
}

/**
 * Upload archive from remote share to local path using localfs
 *
 * @param __fn - name of archive to upload
 * @param __local_path - local path to store archive
 * @return TRUE in success, FALSE otherwise
 */
static BOOL
upload_archive_through_localfs (const char *__fn, const char *__local_path)
{
  static char problems_root[4096] = {0};
  char server_fn[4096], local_fn[4096];
  FILE *istream, *ostream;
  char buf[BUFFER_LENGTH];
  long len;

  if (!problems_root[0])
    {
      INF_PCHAR_KEY (problems_root, "ProblemUploader/ServerProblemsRoot");
    }

  snprintf (server_fn, BUF_SIZE (server_fn), "%s/%s", problems_root, __fn);
  snprintf (local_fn, BUF_SIZE (local_fn), "%s/%s", __local_path, __fn);

  /* Open file descriptor */
  istream = fopen (server_fn, "rb");

  if (!istream)
    {
      return FALSE;
    }

  ostream = fopen (local_fn, "wb");
  if (!ostream)
    {
      fclose (istream);
      return FALSE;
    }

  /* Copy-pasting data from remote to local file */
  do
    {
      len = fread (buf, 1, sizeof (buf), istream);
      if (len > 0)
        {
          fwrite (buf, sizeof (char), len, ostream);
        }
    }
  while (len > 0);

  /* Close file descriptor */
  fclose (istream);
  fclose (ostream);

  unlink (server_fn);

  return TRUE;
}

/**
 * Upload archive from remote share to local path
 *
 * @param __fn - name of archive to upload
 * @param __local_path - local path to store archive
 * @return TRUE in success, FALSE otherwise
 */
static BOOL
upload_archive (const char *__fn, const char *__local_path)
{
  if (upload_from_samba ())
    {
      return upload_archive_through_samba (__fn, __local_path);
    }
  else
    {
      return upload_archive_through_localfs (__fn, __local_path);
    }
}

/**
 * Unpack archive with tests
 *
 * @param __fn - name of archive to unpack
 * @param __tmp_path - path to extract to
 */
static BOOL
unpack_archive (const char *__fn, const char *__tmp_path)
{
  char full[4096];

  snprintf (full, BUF_SIZE (full), "%s/%s", __tmp_path, __fn);
  if (unpack_file (full, __tmp_path))
    {
      return FALSE;
    }

  return TRUE;
}

/****
 * Checkers
 */

/**
 * Get full checkers' directory
 *
 * @param __out - buffer to store directory
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
 * Unlink existing `checker` file in uploading dir
 *
 * @param __tmp_path - path to unlink file in
 */
static void
unlonk_old_checker (const char *__tmp_path)
{
  char full[4096];
  snprintf (full, BUF_SIZE (full), "%s/checker", __tmp_path);
  unlink (full);
}

/**
 * Build command to execute compiler
 *
 * @param __compiler_id - ID of compiler to use
 * @param __source_file - source file to compile
 * @param __cflags - compilator's flags
 * @param __out - buffer to store command
 */
static void
build_compiler_command (const char *__compiler_id, const char *__source_file,
                        const char *__cflags, char *__out)
{
  char *cmd = COMPILER_SAFE_PCHAR_KEY (__compiler_id, "Command", "");

  char flags[4096] = "", flags_path[1024];

  strcpy (__out, cmd);

  snprintf (flags_path, BUF_SIZE (flags_path),
            "Checker/CompilerFlags/%s", __compiler_id);
  INF_PCHAR_KEY (flags, flags_path);

  strcat (flags, " ");
  strcat (flags, __cflags);

  REPLACE_VAR (__out, "flags", flags);
  REPLACE_VAR (__out, "source", __source_file);
  REPLACE_VAR (__out, "output", "checker");
}

/**
 * Execute compilator
 *
 * @param __compiler_id - ID of compiler
 * @param __cmd - command to execute
 * @param __dir - working directory
 * @param __buf - buffer to store compiler's messages
 * @param __err_desc - error description
 * @return TRUE on success, FALSE otherwise
 */
static BOOL
exec_compiler (const char *__compiler_id, const char *__cmd, const char *__dir,
               char *__buf, char *__err_desc)
{
  DWORD compiler_ml;
  DWORD compiler_tl;
  run_process_info_t *proc;
  BOOL result = TRUE;

  /* Get compiler's limits */
  /* Memory limit */
  compiler_ml = COMPILER_SAFE_INT_KEY (__compiler_id, "Limits/RSS",
                                       INFORMATICS_COMPILER_RSS_LIMIT);

  /* Time limit */
  compiler_tl = COMPILER_SAFE_FLOAT_KEY (__compiler_id, "Limits/Time",
                                 INFORMATICS_COMPILER_TIME_LIMIT) * USEC_COUNT;

  /* Create process */
  INF_DEBUG_LOG ("uploader-problem: Executing compiler (cmd: %s)\n", __cmd);
  proc = run_create_process (__cmd, __dir, compiler_ml, compiler_tl);
  run_execute_process (proc); /* Execute process and.. */
  run_pwait (proc); /* ..wait finishing of process */
  INF_DEBUG_LOG ("uploader-problem: Finish executing compiler\n", __cmd);

  if (RUN_PROC_EXEC_ERROR (*proc))
    {
      INF_DEBUG_LOG ("uploader-problem: Fatal error executing compiler: %s\n",
                     __cmd, RUN_PROC_ERROR_DESC (*proc));
      sprintf (__err_desc, "Fatal error executing compiler: %s",
               RUN_PROC_ERROR_DESC (*proc));
      run_free_process (proc);
      return FALSE;
    }

  /* Set output buffer from pipe */
  if (RUN_PROC_PIPEBUF (*proc))
    {
      strcpy (__buf, RUN_PROC_PIPEBUF (*proc));
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
 * Compile checker from source
 *
 * @param __tmp_path - temporary path where problem is uploaded
 * @param __err_desc - buffer for error description
 * @return TRUE on success, FALSE otherwise
 */
static BOOL
compile_checker (const char *__tmp_path, char *__err_desc)
{
  char err[4096];
  char config_fn[4096], cmd[4096] = "", checker_fn[4096];
  dynastruc_t *hive_params = 0;

  char *compiler_id, *source_file, *cflags;

  /* Unlink existing checker */
  unlonk_old_checker (__tmp_path);

  snprintf (checker_fn, BUF_SIZE (checker_fn), "%s/checker", __tmp_path);

  /* Open checker's config file */
  snprintf (config_fn, BUF_SIZE (config_fn), "%s/checker.conf", __tmp_path);

  if (!fexists (config_fn))
    {
      strcpy (__err_desc, "Unable to locate checker's config file");
      return FALSE;
    }

  if (hive_parse_file (config_fn, &hive_params, err))
    {
      sprintf (__err_desc, "Error while parsing checker's config file: %s",
               err);
      return FALSE;
    }

  CHECK_PARAM ("CompilerID");
  CHECK_PARAM ("SourceFile");

  compiler_id = flexval_get_string (hive_open_key (hive_params, "CompilerID"));
  source_file = flexval_get_string (hive_open_key (hive_params, "SourceFile"));
  cflags = flexval_get_string (hive_open_key (hive_params, "CFLAGS"));

  build_compiler_command (compiler_id, source_file, cflags, cmd);
  if (!strcmp (cmd, ""))
    {
      strcpy (__err_desc, "Unable to get compiler's command "
                          "to compile checker");
      return FALSE;
    }

  strcpy (err, "");
  if (!exec_compiler (compiler_id, cmd, __tmp_path, err, __err_desc))
    {
      if (!strcmp (__err_desc, ""))
        sprintf (__err_desc, "Checker's compilation error. "
                             "Message from compiler: %s", err);
      return FALSE;
    }

  if (!fexists (checker_fn))
    {
      strcpy (__err_desc, "Checker's compilation error.");
      if (strcmp (err, ""))
        {
          strcat (__err_desc, " ");
          strcat (__err_desc, err);
        }
      return FALSE;
    }

  dyna_destroy (hive_params, hive_dyna_deleter);

  return TRUE;
}

/**
 * Use pre-compiled checker
 *
 * @param __checker - checker id
 * @param __tmp_path - temporary path where problem is uploaded
 * @param __err_desc - error description
 * @return TRUE on success, FALSE otherwise
 */
static BOOL
use_checker (const char *__checker, const char *__tmp_path, char *__err_desc)
{
  char checkers_dir[4096], full_existing[4096], full_new[4096];
  unlonk_old_checker (__tmp_path);

  full_checkers_dir (checkers_dir);
  snprintf (full_existing, BUF_SIZE (full_existing),
            "%s/%s", checkers_dir, __checker);

  if (!fexists (full_existing))
    {
      strcpy (__err_desc, "Unable to use pre-compiled checker: no such file");
      return FALSE;
    }

  snprintf (full_new, BUF_SIZE (full_new), "%s/checker", __tmp_path);

  symlink (full_existing, full_new);

  return TRUE;
}

/**
 * Compile or use existing checker for problem
 *
 * @param __checker - checer name(id)
 * @param __tmp_path - temporary path where problem is uploaded
 * @param __err_desc - error description
 * @return TRUE on success, FALSE otherwise
 */
static BOOL
checkerize (const char *__checker, const char *__tmp_path, char *__err_desc)
{
  if (!__checker || atol (__checker) <= 0)
    {
      return compile_checker (__tmp_path, __err_desc);
    }
  else
    {
      return use_checker (__checker, __tmp_path, __err_desc);
    }

  return FALSE;
}

/****
 *
 */

/**
 * Validate names of files in archive
 *
 * @param __id - problem ID
 * @param __path - path where files will be validated
 */
static void
validate_file_names (const char *__id, const char *__path)
{
  dynastruc_t *listing;
  char *name;
  char new_name[1024];
  char full_src[4096], full_name[4096];

  /* Get listing of directory */
  listing = dir_listing (__path);

  DYNA_FOREACH (listing, name);
    if (preg_match ("/^([0-9]*\\.(tst|ans)|(checker.conf)|(tests.conf))$/i",
                    name))
      {
        strlowr (name, new_name);

        snprintf (full_name, BUF_SIZE (full_name),
                  "%s/%s", __path, new_name);

        /* Convert test to lowercase */
        if (strcmp (name, new_name))
          {
            INF_LOG ("Renaming file %s to %s in problem %s\n", name,
                     new_name, __id);

            snprintf (full_src, BUF_SIZE (full_name),
                      "%s/%s", __path, name);

            rename (full_src, full_name);
          }
      }
  DYNA_DONE;

  /* Free used memory */
  dyna_destroy (listing, dyna_deleter_free_ref_data);
}

/**
 * Recode tests from one character set to another
 *
 * @param __id - problem ID which tests will be recoded
 * @param __path - path to tests storage
 * @param __from - current tests character set
 * @param __to - convert tests to character set
 */
static void
recode_tests (const char *__id, const char *__path,
              const char *__from, const char *__to)
{
  dynastruc_t *listing;
  char *name;
  char full_name[4096];
  const char *from, *to;
  BOOL guess_cp = FALSE;

#ifdef USE_ENCA
  const char *fallback;
#endif

  if (!strcmp (__from, __to) && strcasecmp (__from, "auto"))
    {
      /* Recoding is not necessary */
      return;
    }

  if (!strcasecmp (__to, "auto"))
    {
      to = get_current_charset ();
    }

  guess_cp = strcasecmp (__to, "auto") == 0;

#ifndef USE_ENCA
  if (guess_cp)
    {
      INF_ERROR ("Trying to recode tests for problem %s from automatic "
                 "character set without ENCA support", __id);
      return;
    }
#else
  /*
   * TODO: May be we should use something different here?
   */
  fallback = get_current_charset ();
#endif

  INF_LOG ("Recoding tests for problem %s from character set %s to %s\n",
           __id, __from, to);

  /* Get listing of directory */
  listing = dir_listing (__path);

  from = __from;

  DYNA_FOREACH (listing, name);
    if (preg_match ("/^[0-9]*\\.(tst|ans)$/i",  name))
      {
        snprintf (full_name, BUF_SIZE (full_name),
                  "%s/%s", __path, name);

#ifdef USE_ENCA
        if (guess_cp)
          {
            from = guess_file_cp (full_name, "ru", fallback);

            INF_LOG ("Recoding test %s of problem %s from character set "
                     "from %s to %s\n", name, __id, from, to);
          }
#endif

        recode_file (full_name, from, to);
      }
  DYNA_DONE;

  /* Free used memory */
  dyna_destroy (listing, dyna_deleter_free_ref_data);
}

/**
 * Adopt files in archive for usage
 *
 * @param __id - problem ID
 * @param __path - path where tests will be adopted
 */
static void
adopt_tests (const char *__id, const char *__path)
{
  char cfg_file[4096], err[4096];
  dynastruc_t *cfg = NULL;

  /* Validate names of files came from archive */
  validate_file_names (__id, __path);

  /* Thin tests configuration */
  snprintf (cfg_file, BUF_SIZE (cfg_file), "%s/tests.conf", __path);
  if (fexists (cfg_file))
    {
      if (!hive_parse_file (cfg_file, &cfg, err))
        {
          flex_value_t *recode_from_flex, *recode_to_flex;

          recode_from_flex = hive_open_key (cfg, "RecodeFrom");
          recode_to_flex = hive_open_key (cfg, "RecodeTo");

          if (recode_from_flex && recode_to_flex)
            {
              char *recode_from, *recode_to;

              recode_from = flexval_get_string (recode_from_flex);
              recode_to = flexval_get_string (recode_to_flex);

              recode_tests (__id, __path, recode_from, recode_to);
            }
        }
      else
        {
          INF_LOG ("Unable to parse tests' config file in problem %s\n", __id);
        }
    }
}

/**
 * Move files from temporary storage to data dir
 *
 * @param __id - id of new problem
 * @param __tmp_path - temporary path where problem is uploaded
 * @param __rm_all_data - remove all data from destination directory
 * @return TRUE on success, FALSE otherwise
 */
static BOOL
move_to_dataroot (const char *__id, const char *__tmp_dir, BOOL __rm_all_data)
{
  char data_dir[4096], full_dst[4096], problems_dir[4096];
  char full_checker[4096];

  /* Some initialization */
  INF_PCHAR_KEY (data_dir, "DataDir");
  INF_PCHAR_KEY (problems_dir, "ProblemsDir");
  snprintf (full_dst, BUF_SIZE (full_dst), "%s/%s/%s",
            data_dir, problems_dir, __id);

  snprintf (full_checker, BUF_SIZE (full_checker), "%s/%s",
            full_dst, "checker");

  if (__rm_all_data)
    {
      /* Unlink previous folder */
      unlinkdir (full_dst);
    }

  unlink (full_checker);

  fmkdir (data_dir, 00770);

  fcopydir (__tmp_dir, full_dst);

  adopt_tests (__id, full_dst);

  return TRUE;
}

/****
 *
 */

/**
 * Uploading stuff
 *
 * @param __params - params of problem
 * @param __err - error code
 * @param __err_desc - description of error
 * @return TRUE on success, FALSE otherwise
 */
static BOOL
upload_task (assarr_t *__params, char *__err, char *__err_desc)
{
  char *a_id = assarr_get_value (__params, "ID");
  char *filename = assarr_get_value (__params, "FILENAME");
  char *checker = assarr_get_value (__params, "CHECKER");

  BOOL archive_attached;

  char uploading_tmp_dir[4096];

  /* Some initialization */
  strcpy (__err_desc, "");

  create_temporary_dir (a_id, uploading_tmp_dir);

  CHECK_ACTIVE ();

  archive_attached = (filename && strcmp (filename, ""));

  if (archive_attached)
    {

      if (!upload_archive (filename, uploading_tmp_dir))
        {
          strcpy (__err, "E_UPLOAD");
          strcpy (__err_desc, "Unable to upload archive");
          goto __fail_;
        }

      CHECK_ACTIVE ();

      if (!unpack_archive (filename, uploading_tmp_dir))
        {
          strcpy (__err, "E_UNPACK");
          strcpy (__err_desc, "Unable to unpack archive");
          goto __fail_;
        }
    }

  if (!checkerize (checker, uploading_tmp_dir, __err_desc))
    {
      strcpy (__err, "E_CHECKER");
      goto __fail_;
    }

  move_to_dataroot (a_id, uploading_tmp_dir, archive_attached);

  unlinkdir (uploading_tmp_dir);

  return TRUE;

__fail_:
  unlinkdir (uploading_tmp_dir);
  return FALSE;
}

/**
 * Send result of uploading to WebIFACE
 *
 * @param __params - params of problem
 * @param __err - error code
 * @param __desc - error description
 */
static void
put_problem (assarr_t *__params, const char *__err, const char *__desc)
{
  static char url_prefix[4096];
  static BOOL initialized = FALSE;

  char url[4096], desc[65536];

  http_message_t *msg;

  CHECK_ACTIVE_VOID ();

  if (!__desc) __desc = "";

  if (!strcmp (__err, "OK"))
    {
      INF_INFO ("Uploaded problem %s@0\n",
                (char*) assarr_get_value (__params, "ID"));
    }
  else
    {
      INF_ERROR ("Error uploading problem %s@0\n",
                 (char*) assarr_get_value (__params, "ID"));
    }

  if (!initialized)
    {
      wt_transport_prepare_url ("put_problem", url_prefix);
      initialized = TRUE;
    }

  urlencode (__desc, desc);

  snprintf (url, BUF_SIZE (url), "%s&id=%s&lid=0&err=%s&desc=%s", url_prefix,
            (char*) assarr_get_value (__params, "ID"), __err, desc);

  msg = wt_transport_send_message (url);

  if (!msg)
    {
      return;
    }

  if (!HTTP_STATUS_OK (*msg))
    {
      INF_ERROR ("Error sending HTTP request for return problem's "
                 "uploading result: %d\n", HTTP_STATUS (*msg));
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
 * Task uploadin thread
 */
static gpointer
uploading_thread (gpointer __unused)
{
  assarr_t *params;

  params = assarr_create ();

  if (active && send_ipc_request (params))
    {
      if (assarr_isset (params, "ID"))
        {
          char err[1024], desc[65536];
          INF_DEBUG_LOG ("uploader-problem: Start uploading problem\n");
          Informatics_SuspendTesting ();
          if (check_params (params))
            {
              strcpy (err, "OK");
              if (upload_task (params, err, desc))
                {
                  put_problem (params, err, "");
                }
              else
                {
                  put_problem (params, err, desc);
                }
            }
          else
            {
              put_problem (params, "ERR", "Required parameter is undefined");
            }
          Informatics_ResumeTesting ();
        }
    }
  assarr_destroy (params, assarr_deleter_free_ref_data);

  thread = 0;

  g_mutex_unlock (mutex);

  g_thread_exit (0);
  return 0;
}

/**
 * Upload problem callback
 *
 * @return zero on success, non-zero otherwise
 */
int
Informatics_UploadProblem (void *__unused, void *__call_unused)
{

  if (!mutex)
    {
      mutex = mutex_create ();
      if (!mutex)
        {
          return -1;
        }
    }

  if (g_mutex_trylock (mutex))
    {
      if (upload_from_samba () && !samba_initialized ())
        {
          /* If uploading should be from SAMBA and */
          /* SAMBA is not initialized */
          return 0;
        }

      thread = g_thread_create (uploading_thread, 0, FALSE, 0);
    }

  return 0;
}

/**
 * Stop problem uploading
 *
 * @return zero on success, non-zero otherwise
 */
int
Informatics_StopProblemUploading (void *__unused, void *__call_unused)
{
  active = FALSE;
  if (mutex)
    {
      /*  Wait for mutex unlock */
      mutex_lock (mutex);

      /* Safe freeing of mutex */
      mutex_unlock (mutex);
      mutex_free (mutex);
      mutex = 0;
    }
  return 0;
}
