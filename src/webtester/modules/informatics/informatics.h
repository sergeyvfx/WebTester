/**
 * WebTester Server - server of on-line testing system
 *
 * Testing module for simple olympiands
 *
 * Copyright 2008 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#ifndef _WT_INFORMATICS_H_
#define _WT_INFORMATICS_H_

#include <webtester/autoinc.h>

BEGIN_HEADER

#include <webtester/task.h>
#include <webtester/library.h>
#include <libwebtester/flexval.h>

#define INFORMATICS_MAJOR_VERSION  1
#define INFORMATICS_MINOR_VERSION  1
#define INFORMATICS_LIBNAME        "Informatics"

#define INFORMATICS_MAX_THREADS             10

#define INFORMATICS_COMPILER_RSS_LIMIT      32768
#define INFORMATICS_COMPILER_TIME_LIMIT     30  /* secs */
#define INFORMATICS_CHECKER_RSS_LIMIT       32768
#define INFORMATICS_CHECKER_TIME_LIMIT      30  /* secs */

#define INFORMATICS_UNLINK_INTERVAL         10 /* secs */
#define INFORMATICS_KEEP_ALIVE_TESTDIRS     100

#define INFORMATICS_EXEC            "solution"
#define INFORMATICS_EXECEXT         ""
#define INFORMATICS_SOURCE          "solution"
#define INFORMATICS_SRCEXT          ".sol"

#define INFORMATICS_TSTEXT          ".tst"
#define INFORMATICS_ANSEXT          ".ans"

#define INFORMATICS_ENABLE_REPORT    1

#define INFORMATICS_CHECKER_UPLOAD_INTERVAL 0.5 /* secs */

/* Initialize testing stuff */
BOOL
Informatics_init_testing (void);

/* Wait for stopping all testing threads */
int
Informatics_stop_testing (void *__unused, void *__call_unused);

/* Uninitialize testing stuff */
void
Informatics_done_testing (void);

/* Creates new testing thread */
BOOL
Informatics_start_testing_thread (wt_task_t *__task, char *__error);

/****
 *
 */

/* Parameter in task's compiler tree */
flex_value_t*
Informatics_compiler_config_val (const char *__compiler_id,
                                 const char *__path);

/* Parameter in compiler tree */
flex_value_t*
Informatics_compiler_common_val (const char *__path);

/****
 *
 */

int
Informatics_UploadProblem (void *__unused, void *__call_unused);

int
Informatics_UploadChecker (void *__unused);

int
Informatics_StopCheckerUploading (void *__unused, void *__call_unused);

int
Informatics_StopProblemUploading (void *__unused, void *__call_unused);

int
Informatics_SuspendTesting (void);

int
Informatics_ResumeTesting (void);

/****
 * IPC stuff
 */

/* Initialize Informatics IPC stuff */
int
Informatics_ipc_init (void);

/* Uninitialize Informatics IPC stuff */
int
Informatics_ipc_done (void);

END_HEADER

#endif
