/*
 *
 * ================================================================================
 *  strlib.h
 * ================================================================================
 *
 *  String library. Includes some powerfull functions to work with strings.
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#ifndef strlib_h
#define strlib_h

void
strupr                             (char *__src, char *__dest);

void
strlowr                            (char *__src, char *__dest);

int
strlastchar                        (char *__str, char __ch);

void
strsubstr                          (char *__src, int __start, int __len, char *__out);

long
explode                            (char *__s, char *__separator, char ***__out);

void
free_explode_data                  (char **__self);

void
addslashes                         (char *__self, char *__out);

void
stripslashes                       (char *__self, char *__out);

void
strarr_append                      (char ***__arr, char *__s, int *__count);

void
strarr_free                        (char **__arr, int __count);

char*
realloc_string                     (char *__s, int __delta);

void
trim                               (char *__self, char *__out);

#endif
