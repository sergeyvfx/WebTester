/*
 *
 * ================================================================================
 *  informatics.c - part of the WebTester Server
 * ================================================================================
 *
 *  Testing module for simple olympiands.
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#ifndef _wt_informatics_h_
#define _wt_informatics_h_

#include <webtester/autoinc.h>
#include <webtester/task.h>
#include <webtester/library.h>
#include <libwebtester/flexval.h>

#define INFORMATICS_MAJOR_VERSION  1
#define INFORMATICS_MINOR_VERSION  0
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

BOOL            // Initialize testing stuff
Informatics_init_testing           (void);

int             // Wait for stopping all testing threads
Informatics_stop_testing           (void *__unused);

void            // Uninitialize testing stuff
Informatics_done_testing           (void);

BOOL            // Creates new testing thread
Informatics_start_testing_thread   (wt_task_t *__task, char *__error);

////////
//

flex_value_t*   // Parameter in task's compiler tree
Informatics_compiler_config_val    (char *__compiler_id, char *__path);

flex_value_t*
Informatics_compiler_common_val    (char *__path);

////////
//

int
Informatics_UploadProblem          (void *__unused);

int
Informatics_UploadChecker          (void *__unused);

int
Informatics_StopCheckerUploading   (void *__unused);

int
Informatics_StopProblemUploading   (void *__unused);

#endif
