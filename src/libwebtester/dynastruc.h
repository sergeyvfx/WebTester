/*
 *
 * ================================================================================
 *  DYNASTRUC
 * ================================================================================
 *
 *  Dynamic structures stuff (stack, queues, deks).
 *  Practically stable. I hope :)
 *  
 *  Written (by Nazgul) under General Public License.
 *
*/

#ifndef dyna_h
#define dyna_h

#define dyna_append(self,data,tag) dyna_add_to_back (self,data,tag)
#define dyna_length(self) ( (self)?((self)->count):(-1) )
#define dyna_head(self) ( (self)?((self)->head):(0) )
#define dyna_tail(self) ( (self)?((self)->tail):(0) )
#define dyna_next(cur)  ( (dyna_item_t*)((cur)?((cur)->next_ptr):(0)) )
#define dyna_prev(cur)  ( (dyna_item_t*)((cur)?((cur)->prev_ptr):(0)) )
#define dyna_data(cur)  ( (cur)?((cur)->data):(0))

#define DYNA_FOREACH(__dyna, __data) \
  { \
    dynastruc_t *__cur_dyna_; \
    __cur_dyna_=__dyna; \
    dyna_item_t *__cur_, *__next_; \
    __cur_=dyna_head (__dyna); \
    while (__cur_) \
      { \
        __data=dyna_data (__cur_); \
        __next_=dyna_next (__cur_);

#define DYNA_BREAK break

#define DYNA_DONE \
        __cur_=__next_; \
      } \
  }

#define DYNA_DELETE_CUR(__deleter) \
  dyna_delete (__cur_dyna_, __cur_, __deleter)

typedef struct
{
  void *data;
  int tag;
  void *next_ptr;
  void *prev_ptr;
} dyna_item_t;

typedef struct 
{
 dyna_item_t *head, *tail;
 dyna_item_t *find_data;
 long count;
} dynastruc_t;


typedef void (*dyna_deleter)    (void* item);
typedef int  (*dyna_comparator) (void *left, void *right);

// Basic properties
dynastruc_t*
dyna_create                   (void);

int
dyna_destroy                  (dynastruc_t *__self, dyna_deleter __deleter );

int
dyna_delete_all               (dynastruc_t *__self, dyna_deleter __deleter);

int
dyna_delete                   (dynastruc_t *__self, dyna_item_t *__item, dyna_deleter __deleter);

void
dyna_deleter_free_ref_data         (void *__self);

int
dyna_search_reset             (dynastruc_t *__self);

dyna_item_t*
dyna_search                   (dynastruc_t *__self, void *__data, int __tag,  dyna_comparator __comparator);

// Stack properties
int
dyna_push                     (dynastruc_t *__self, void *__data, int __tag);

int
dyna_pop                      (dynastruc_t *__self, void **__data, int *__tag);

// Queue properties
int
dyna_add_to_front             (dynastruc_t *__self, void *__data, int __tag);

int
dyna_add_to_back              (dynastruc_t *__self, void *__data, int __tag);

int
dyna_del_from_front           (dynastruc_t *__self, void **__data, int *__tag);

int
dyna_del_from_back            (dynastruc_t *__self, void **__data, int *__tag);
                                   
int
dyna_empty                    (dynastruc_t *__self);

dyna_item_t*
dyna_get_item_by_index        (dynastruc_t *__self, int __i);

void
dyna_sort                     (dynastruc_t *__self, dyna_comparator __comparator);

// Default comparators
int
dyna_string_comparator        (void *__l, void *__r);

int
dyna_eq_comparator            (void *__l, void *__r);

int
dyna_sort_listing_comparator  (void *__l, void *__r);

#endif
