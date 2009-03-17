/**
 * WebTester Server - server of on-line testing system
 *
 * IPC stuff
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include "informatics.h"
#include "macros.h"

#include <malloc.h>

#include <libwebtester/ipc.h>
#include <libwebtester/util.h>
#include <libwebtester/fs.h>
#include <libwebtester/assarr.h>

#define READ_FILE_ITER(ext, arr) \
  snprintf (full, BUF_SIZE (full),"%s%s", prefix, ext); \
  stream = fopen (full, "rb"); \
  if (!stream) \
    { \
      INF_DEBUG_LOG ("IPC: Error reading contents of file %s", full); \
      err = 1; \
      break; \
    } \
  size = fread (buf, 1, max_size, stream); \
  buf[size] = 0; \
  assarr_set_value (arr, pchar, strdup (buf)); \
  fclose (stream);

/**
 * Get format to use for test file building
 *
 * @param __data_dir - full data directory of problem
 * @param __from - LO boundary of needed range
 * @param __to - HI boundary of needed range
 * @return mask of tests file
 */
static char*
get_test_format (const char *__data_dir, long __from, long __to)
{
  char dummy[4096];
  char tst_ext[128];

  if (__to > 100)
    {
      return "%03ld";
    }

  INF_PCHAR_KEY (tst_ext, "Tests/InputExtension");

  snprintf (dummy, BUF_SIZE (dummy), "%s/%02ld%s", __data_dir, __from,
    tst_ext);

  if (fexists (dummy))
    {
      return "%02ld";
    }

  return "%03ld";
}

/**
 * Pack contents of test files for sending through IPC
 *
 * @param __data_dir - full problem storage directory
 * @param __test_format - format of test files
 * @param __from - from which tests pack
 * @param __to - to which tests pack
 * @return packed array wiith tests
 * @sizeeffect allocate memory for output value
 */
static char*
pack_tests_for_problem (const char *__data_dir, const char *__test_format,
                        long __from, long __to)
{
  char *res = NULL, *dummy;
  long cur, max_size, size;
  char pchar[64], prefix[4096], full[4096];
  char tst_ext[128], ans_ext[128];
  FILE *stream;
  char *buf;
  int err = 0;
  assarr_t *tsts, *anss, *arr;

  INF_PCHAR_KEY (tst_ext, "Tests/InputExtension");
  INF_PCHAR_KEY (ans_ext, "Tests/OutputExtension");

  INF_SAFE_INT_KEY (max_size, "MaxTestSendSize", 0);

  if (max_size <= 0)
    {
      return NULL;
    }

  buf = malloc (max_size);

  tsts = assarr_create ();
  anss = assarr_create ();

  for (cur = __from; cur <= __to; ++cur)
    {
      snprintf (pchar, BUF_SIZE (pchar), __test_format, cur);
      snprintf (prefix, BUF_SIZE (prefix), "%s/%s", __data_dir, pchar);

      snprintf (pchar, BUF_SIZE (pchar), "%ld", cur);

      READ_FILE_ITER (tst_ext, tsts);
      READ_FILE_ITER (ans_ext, anss);
    }

  free (buf);

  if (err)
    {
      assarr_destroy (tsts, assarr_deleter_free_ref_data);
      assarr_destroy (anss, assarr_deleter_free_ref_data);
      return NULL;
    }

  arr = assarr_create ();

  assarr_pack (tsts, &dummy);
  assarr_set_value (arr, "tst", dummy);

  assarr_pack (anss, &dummy);
  assarr_set_value (arr, "ans", dummy);

  assarr_pack (arr, &res);

  assarr_destroy (tsts, assarr_deleter_free_ref_data);
  assarr_destroy (anss, assarr_deleter_free_ref_data);
  assarr_destroy (arr, assarr_deleter_free_ref_data);

  return res;
}

/**
 * Get tests from problem
 *
 * @param __problem_id - ID of problem to get tests of
 * @param __range - range of tests to get
 * @return zero on success, non-zero otherwise
 */
static int
get_tests_ipc (const char *__problem_id, const char *__range)
{
  char data_dir[4096], problems_dir[4096];
  char full_data_dir[4096];
  char *tst_format, *data;
  long from, to;

  if (parse_range (__range, &from, &to) || from > to)
    {
      IPC_PROC_ANSWER ("-ERR Invalid range\n");
      return -1;
    }

  INF_DEBUG_LOG ("Packing tests %ld-%ld for problem %s\n",
    from, to, __problem_id);

  INF_PCHAR_KEY (data_dir, "DataDir");
  INF_PCHAR_KEY (problems_dir, "ProblemsDir");

  snprintf (full_data_dir, BUF_SIZE (data_dir), "%s/%s/%s", data_dir,
    problems_dir, __problem_id);

  tst_format = get_test_format (full_data_dir, from, to);
  INF_DEBUG_LOG ("Format for tests from problen %s: %s\n",
    __problem_id, tst_format);

  data = pack_tests_for_problem (full_data_dir, tst_format, from, to);

  if (data)
    {
      IPC_PROC_ANSWER ("+OK %ld\n%s\n", strlen (data), data);

      free (data);

      return 0;
    }

  IPC_PROC_ANSWER ("-ERR\n");

  return 0;
}

/**
 * Handler of IPC command informatics
 *
 * @param __argc - arguments count
 * @param __argv - argument values
 */
static int
informatics_ipc (int __argc, char **__argv)
{
  IPC_ADMIN_REQUIRED

  if (__argc == 4 && !strcmp (__argv[1], "get_tests"))
    {
      return get_tests_ipc (__argv[2], __argv[3]);
    }
  else
    {
      goto __usage_;
    }

  return 0;

__usage_:
  IPC_PROC_ANSWER ("-ERR Usage: `informatics` get_tests "
                   "<problem-id> <tests-range>\n");
  return 0;
}

/********
 * User's backend
 */

/**
 * Initialize Informatics IPC stuff
 *
 * @return zero on success, non-zero otherwise
 */
int
Informatics_ipc_init (void)
{
  ipc_proc_register ("informatics", informatics_ipc);
  return 0;
}

/**
 * Uninitialize Informatics IPC stuff
 *
 * @return zero on success, non-zero otherwise
 */
int
Informatics_ipc_done (void)
{
  ipc_proc_unregister ("informatics");
  return 0;
}
