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

#include "libgda.h"
#include "utf8.h"
#include "locale.h"

#include <string.h>

int
libgda_get_errors                 (GdaConnection *connection)
{
/*  GList *list;
  GList *node;
  GdaError *error;
 
  list=(GList*)gda_connection_get_errors (connection);
  for (node=g_list_first (list); node!=NULL; node=g_list_next (node))
    {
      error=(GdaError *) node->data;
      printf ("Error no: %ld\n", gda_error_get_number (error));
      printf ("desc: %s\n", gda_error_get_description (error));
      printf ("source: %s\n", gda_error_get_source (error));
      printf ("sqlstate: %s\n", gda_error_get_sqlstate (error));
    }*/
  return 0;
}

int
libgda_execute_sql_non_query      (GdaConnection *__connection, const char *__buffer)
{
  GdaCommand *command;
  gint number;
  command=gda_command_new (__buffer, GDA_COMMAND_TYPE_SQL, GDA_COMMAND_OPTION_STOP_ON_ERRORS);
  number=gda_connection_execute_non_query (__connection, command, NULL);
  libgda_get_errors (__connection);
  gda_command_free (command);
  return (number);
}

int
libgda_show_table                 (GdaDataModel * __dm, void *__reserved)
{
  gint row_id;
  gint column_id;
  const GdaValue *value;

  for (column_id=0; column_id<gda_data_model_get_n_columns (__dm); column_id++)
    printf ("%s\t", gda_data_model_get_column_title (__dm, column_id));
  printf ("\n");
          
  for (row_id=0; row_id<gda_data_model_get_n_rows (__dm); row_id++)
    {
      for (column_id=0; column_id<gda_data_model_get_n_columns (__dm); column_id++)
        {
          char *str;
          value=gda_data_model_get_value_at (__dm, column_id, row_id);
          str=gda_value_stringify ((GdaValue*)value);
          printf ("%s\t", str);
          free (str);
        }
      printf ("\n");
    }
  return 0;
}

int
libgda_execute_sql_command_reload (
                                    GdaConnection *__connection,
                                    const char *__buffer,
                                    libgda_command_handler_t __handler,
                                    void *__data
                                  )
{
  GdaCommand *command;
  GList *list=NULL;
  GList *node;
  gboolean errors=FALSE;
  int res=0;
  GdaDataModel *dm;
  command=gda_command_new (__buffer, GDA_COMMAND_TYPE_SQL, GDA_COMMAND_OPTION_STOP_ON_ERRORS);
//  dm=gda_connection_execute_single_command (__connection, command, NULL);
  list=gda_connection_execute_command (__connection, command, NULL);
  if (list!=NULL)
    {
      for (node=g_list_first(list); node!=NULL; node=g_list_next (node))
        {
          dm=(GdaDataModel*)node->data;
          if (!dm)
            errors=TRUE; else
            {
              if (__handler) res=__handler (dm, __data);
              g_object_unref (dm);
//              g_free (dm);
            }
        }
      g_list_free (list);
    } else errors=TRUE;
  libgda_get_errors (__connection);
  gda_command_free (command);
  return res;
}

int
libgda_execute_sql_command        (GdaConnection *__connection, const char *__buffer)
{
  return libgda_execute_sql_command_reload (__connection, __buffer, /*libgda_show_table*/0, 0);
}

int
libgda_fill_assarr_row            (GdaDataModel * __dm, void *__data)
{
  static char title[65536];
  libgda_query_output_t *__out=__data;
  assarr_t **sdata;
  if (!__data) return 0;
  gint row_id, column_id;
  gint nrows, ncolumns;
  int i;
  const GdaValue *value;
  nrows=gda_data_model_get_n_rows (__dm); ncolumns=gda_data_model_get_n_columns (__dm);
  if (nrows<=0) return 0;
  sdata=__out->data;
  if (!__out->data) __out->data=malloc (__out->count+nrows);
  for (i=0; i<__out->count; i++) __out->data[i]=sdata[i];
  free (sdata);
  for (row_id=0; row_id<nrows; row_id++)
    {
      __out->data[ __out->count+row_id]=assarr_create ();
      for (column_id=0; column_id<ncolumns; column_id++)
        {
          char *str;
          strcpy (title, gda_data_model_get_column_title (__dm, column_id));
          value=gda_data_model_get_value_at (__dm, column_id, row_id);
          str=gda_value_stringify ((GdaValue*)value);
          assarr_set_value (__out->data[ __out->count+row_id], title, recode_database_data (str));
          g_free (str);
        }
    }
  __out->count+=nrows;
  return 0;
}

int
libgda_execute_query              (GdaConnection *__connection, const char *__buffer, libgda_query_output_t *__out)
{
  if (__out) {__out->count=0; __out->data=0;}
  return libgda_execute_sql_command_reload (__connection, __buffer, libgda_fill_assarr_row, __out);
}

int
libgda_free_query_output          (libgda_query_output_t *__self)
{
  int i;
  for (i=0; i<__self->count; i++)
    assarr_destroy (__self->data[i], assarr_deleter_free_ref_data);
  return 0;
}
