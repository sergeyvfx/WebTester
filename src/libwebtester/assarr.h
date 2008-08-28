/*
 *
 * ================================================================================
 *  assarr.h
 * ================================================================================
 *
 *  Assaciative arrays module
 *
 *  Written (by Nazgul) under General Public License.
 *
*/


#ifndef _assarr_h_
#define _assarr_h_

////////////////////////////////////////
// Type defenitions

typedef struct
{
  char *key;
  void *value;
  void *next_ptr;
} assarr_entry_t;

typedef struct
{
  long m;
  long k;
  int count;
  assarr_entry_t **data;
} assarr_t;

typedef void (*assarr_deleter)     (void *__item);

////////////////////////////////////////
// Macroses

#define ASSARR_FOREACH_DO(__self, __key, __value) \
 { \
  int i; \
  assarr_entry_t *cur; \
  for (i=0; i<(__self).m; i++) \
    { \
      cur=(__self).data[i]; \
      while (cur) \
        { \
          __key=cur->key; \
          __value=cur->value; \
          {
          
#define ASSARR_FOREACH_DONE \
          } \
          cur=cur->next_ptr; \
        } \
    } \
 }

void            // Assarr deleter with freeing of each elements
assarr_deleter_free_ref_data       (void *__item);

assarr_t*       // Create new ass. array
assarr_create                      (void);

void            // Destroy ass. array
assarr_destroy                     (assarr_t *__self, assarr_deleter __deleter);

void*           // Get value by key
assarr_get_value                   (assarr_t *__self, char *__key);

int             // Check for set
assarr_isset                       (assarr_t *__self, char *__key);

void            // Set value by key
assarr_set_value                   (assarr_t *__self, char *__key, void *__value);

int             // Unset value by key
assarr_unset_value                 (assarr_t *__self, char *__key, assarr_deleter __deleter);

// !!! WARNING !!! Works ONLY with PCHAR data
void            // Pack assarr PCHAR data to string
assarr_pack                        (assarr_t *__self, char **__out);

void            // Unpack string to assaciative array
assarr_unpack                      (char *__data, assarr_t *__arr);

#endif
