/*
 *
 * ================================================================================
 *  util.h
 * ================================================================================
 *
 *  Some usefull procedures
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#ifndef util_h
#define util_h

#include <libwebtester/smartinclude.h>
#include <sys/time.h>

int
is_number                          (char *__self);

int
is_integer                         (char *__self);

void
drop_triling_zeroes                (char *__self);

int
fname                              (char *__full, char *__out);

int
dirname                            (char *__full, char *__out);

int
dropextension                      (char *__fn, char *__out);

int
getextension                       (char *__fn, char *__out);

int
sys_launch                         (char *__args, ...);

timeval_t
now                                (void);

timeval_t
timedist                           (timeval_t __from, timeval_t __to);

timeval_t
timedistnow                        (timeval_t __from);

int
tv_usec_cmp                        (timeval_t __tv, __u64_t __usec);

void
prepare_cmdarg                     (char *__src, char *__dst);

long
uid_by_name                        (char *__name);

long
gid_by_name                        (char *__name);

void
get_datetime_strf                  (char *__out, int __size, char *__format);

int
is_truth                           (char *__self);

#endif
