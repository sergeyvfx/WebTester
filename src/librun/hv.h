/**
 * WebTester Server - server of on-line testing system
 *
 * This module contains hypervisor under all accounting informatin,
 * going from kernel space to user space.
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#ifndef _RUN_HYPERVISOR_H_
#define _RUN_HYPERVISOR_H_

#include <libwebtester/smartinclude.h>

BEGIN_HEADER

#include <linux/types.h>
#include <linux/taskstats.h>

/********
 * Constants
 */

/* Default CPU mask */
#define RUN_HV_CPUMASK              "0"

/* Default delay in listening thread. secs. */
#define RUN_HV_DELAY                0.01

/* Default size of one array in HV pool stuff */
#define RUN_HV_POOL_ARR_SIZE        512

/* Default lifetime of info in pool. secs. */
#define RUN_HV_POOL_LIFETIME        10

/* Default delay in GetStats delay. secs. */
#define RUN_HV_POOL_GETSTATS_DELAY        0.2

/* Default count of tries in GetStats stuff */
#define RUN_HV_POOL_GETSTATS_MAX_COUNTER  15

/********
 * Type defenintions
 */

/* Item of hypervisor pool */
typedef struct
{
  /* Timestamp of item */
  int timestamp;

  /* Task information */
  struct taskstats t;
} run_hvpool_item_t;

/* Pool array */
typedef struct
{
  /* Count of items in array */
  int count;

  /* Size of array (maximal capacity) */
  int size;

  /* Data stored in the array */
  run_hvpool_item_t *data;
} run_hvpool_arr_t;

/*******
 *
 */

/* Initialize hypervisor stuff */
int
run_hypervisor_init (void);

/* Uninitialize HyperVisor stuff */
void
run_hypervisor_done (void);

/****
 * POOL
 */

/* Initialize hupervisor pool stuff */
int
run_hvpool_init (void);

/* Uninitialize hypervisor pool stuff */
void
run_hvpool_done (void);

/* Put data to hypervisor pool */
void
run_hvpool_put (const struct taskstats *__self);

/* Get ACCT stats by PID of finished process */
BOOL
run_hvpool_stats_by_pid (__u32 __pid, struct taskstats *__stats);

/* Get ACCT stats by PID of running process */
int
run_hv_proc_stats (__u32 __pid, struct taskstats *__stats);

END_HEADER

#endif
