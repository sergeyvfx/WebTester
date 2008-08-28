/*
 *
 * ================================================================================
 *  gda.h
 * ================================================================================
 *
 *  Gnome Data Access stuff module
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#ifndef _libgda_h_
#define _libgda_h_

#include <libwebtester/assarr.h>
#include <libgda/libgda.h>
#include <glib.h>

typedef int (*libgda_command_handler_t) (GdaDataModel*, void*);

typedef struct
{
  assarr_t **data;
  long count;
} libgda_query_output_t;

int
libgda_execute_sql_non_query      (GdaConnection *__connection, const char *__buffer);

int
libgda_show_table                 (GdaDataModel *__dm, void *__data);

int
libgda_execute_sql_command        (GdaConnection *__connection, const char *__buffer);

int
libgda_execute_sql_command_reload (
                                    GdaConnection *__connection,
                                    const char *__buffer,
                                    libgda_command_handler_t __handler,
                                    void *__data
                                  );

int
libgda_execute_query              (GdaConnection *__connection, const char *__buffer, libgda_query_output_t *__out);

int
libgda_free_query_output          (libgda_query_output_t *_self);

#endif
