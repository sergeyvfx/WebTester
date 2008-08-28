/*
 *
 * ================================================================================
 *  utf8.h
 * ================================================================================
 *
 *  This module contains utf-8 stuff and some other stuff to work with charsets.
 *  (Based on the source of X MultiMedia Systems)
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#ifndef _utf8_h_
#define _utf8_h_

char*
get_current_charset                (void);

char*
convert_string                     (const char *__string, char *__from, char *__to);

char*
convert_to_utf8                    (const char *__string);

char*
convert_from_utf8                  (const char *__string);

char*
recode                             (const char *__string, char *__from, char *__to);

#endif
