/*
 *
 * ================================================================================
 *  acct_hv.c - part of the LibRUN
 * ================================================================================
 *
 *  This module contains hypervisor under all accounting informatin,
 *  going from kernel space to user space.
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#ifndef _run_hypervisor_h_
#define _run_hypervisor_h_

#include <libwebtester/smartinclude.h>

#include <linux/types.h>
#include <linux/taskstats.h>

// Default CPU mask
#define RUN_HV_CPUMASK              "0"

// Default delay in listening thread. secs.
#define RUN_HV_DELAY                0.01

// Default size of one array in HV pool stuff
#define RUN_HV_POOL_ARR_SIZE        512

// Default lifetime of info in pool. secs.
#define RUN_HV_POOL_LIFETIME        10

// Default delay in GetStats delay. secs.
#define RUN_HV_POOL_GETSTATS_DELAY        0.2

// Default count of tries in GetStats stuff
#define RUN_HV_POOL_GETSTATS_MAX_COUNTER  15

////////
// Type defenintions

typedef struct {
  int timestamp;
  struct taskstats t;
} run_hvpool_item_t;

typedef struct {
  int count;
  int size;
  run_hvpool_item_t *data;
} run_hvpool_arr_t;

////////////////
//

int             // Initialize HyperVisor stuff
run_hypervisor_init                (void);

void            // Uninitialize HyperVisor stuff
run_hypervisor_done                (void);

////////////////
// POOL

int             // Initialize HV pool stuff
run_hvpool_init                    (void);

void            // Uninitialize HV pool stuff
run_hvpool_done                    (void);

void            // Put data to HV pool
run_hvpool_put                     (struct taskstats *__self);

BOOL            // Get ACCT stats by PID of finished process
run_hvpool_stats_by_pid            (__u32 __pid, struct taskstats *__stats);

////////////////
//

int             // Get ACCT stats by PID of running process
run_hv_proc_stats                  (__u32 __pid, struct taskstats *__stats);

#endif
