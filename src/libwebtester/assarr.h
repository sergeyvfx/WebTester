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
  int __i_; \
  assarr_entry_t *__cur_, *__next_; \
  for (__i_=0; __i_ < (__self)->m; __i_++) \
    { \
      __cur_=(__self)->data[__i_]; \
      while (__cur_) \
        { \
          __key=__cur_->key; \
          __value=__cur_->value; \
          __next_=__cur_->next_ptr; \
          {
          
#define ASSARR_FOREACH_DONE \
          } \
          __cur_=__next_; \
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

int             // Unset all values
assarr_unset_all                   (assarr_t *__self, assarr_deleter __deleter);

// !!! WARNING !!! Works ONLY with PCHAR data
void            // Pack assarr PCHAR data to string
assarr_pack                        (assarr_t *__self, char **__out);

void            // Unpack string to assaciative array
assarr_unpack                      (char *__data, assarr_t *__arr);

#endif
