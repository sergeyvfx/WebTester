/**
 * WebTester Server - server of on-line testing system
 *
 * HIVE datastruct (tree) stuff.
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include "hive.h"
#include "fs.h"
#include "util.h"
#include "strlib.h"

#include <malloc.h>
#include <string.h>

#define ERROR(__text,__args...) \
  if (__error) {\
    sprintf (__error, __text, ##__args); printf ("## %s\n", __error); }

static long line;

/********
 * General built-in
 */

/**
 * Deleter for hive tree node
 *
 * @param __self - node to delete
 */
void
hive_dyna_deleter (void *__self)
{
  hive_item_t *ptr;
  if (__self)
    {
      ptr = (hive_item_t*) __self;
      free (ptr->header.name);
      free (ptr->header.value.pchar);
      if (ptr->nodes)
        {
          dyna_destroy (ptr->nodes, hive_dyna_deleter);
        }
      free (ptr);
    }
}

/**
 * Comparator of hive node name and given name
 *
 * @param __l - hive node descriptor
 * @param __r - name to compare hive node with
 * @return zero if names are different, non-zero otherwise
 */
int
hive_key_comparator (void *__l, void *__r)
{
  hive_item_t *left_op = __l;
  if (!strcmp (left_op->header.name, __r))
    {
      return 1;
    }
  else
    {
      return 0;
    }
}

/**
 * Spawn new node of hive tree
 *
 * @param __parent - parent of new node
 * @param __owner - owner of new node
 * @param __name - name of node
 * @param __value - value of node
 * @param __flags - node's flags
 * @return descriptor of new hive node f succeed, NULL otherwise
 */
hive_item_t*
hive_spawn_new_node (hive_item_t *__parent, hive_item_t *__owner,
                     char *__name, char *__value, int __flags)
{
  hive_item_t *new_ptr;
  if (!__name)
    {
      return NULL;
    }
  new_ptr = malloc (sizeof (hive_item_t));
  flexval_create (&new_ptr->header.value);

  if (__value)
    {
      /* Smart setting of variable */
      if (__flags & HPF_STRING)
        {
          /* Forced string value */
          flexval_set_string (&new_ptr->header.value, __value);
        }
      else
        if (__flags & HPF_ARRAY) /* Array */
        {
          char dummy[65536];
          flex_value_t *ptr;
          snprintf (dummy, BUF_SIZE (dummy), "[%s]", __value);
          ptr = flexval_unserialize (dummy);
          flexval_set_array (&new_ptr->header.value,
                             (flex_value_t**) FLEXVAL_ARRAY_DATA (ptr));
          flexval_free (ptr);
        }
      else
        if (is_integer (__value))
          {
            /* Integer value */
            flexval_set_int (&new_ptr->header.value, flexval_atol (__value));
          }
      else
        if (is_number (__value))
          {
            /* Float value */
            flexval_set_float (&new_ptr->header.value,
                               flexval_atolf (__value));
          }
        else
          {
            flexval_set_string (&new_ptr->header.value,
                                __value); /* Set value as string */
          }
    }
  else
    {
      flexval_set_string (&new_ptr->header.value, "");
    }

  new_ptr->header.name = strdup (__name);

  new_ptr->parent = __parent;
  new_ptr->owner = __owner;
  new_ptr->nodes = 0;
  if (__parent && __parent->nodes)
    {
      new_ptr->index = dyna_length (__parent->nodes);
    }
  else
    {
      new_ptr->index = 0;
    }

  return new_ptr;
}

/**
 * Add variable to hive node
 *
 * @param __self - in which node variable will be added
 * @param __name - name of variable
 * @param __value - value of variable
 * @param __flags - variable's flags
 * @return zero on success, non-zero otherwise
 */
int
hive_add_variable (hive_item_t *__self, char *__name,
                   char *__value, int __flags)
{
  hive_item_t *new_node;

  if (!__self || !__name)
    {
      return -1;
    }

  if (!__self->nodes)
    {
      __self->nodes = dyna_create ();
    }

  new_node = hive_spawn_new_node (__self, 0, __name, __value, __flags);
  dyna_add_to_back (__self->nodes, new_node, 0);
  return 0;
}

/**
 * Copy childs from one hive node to another
 *
 * @param __dst - destination hive node
 * @param __src - source hive node
 */
int
hive_copy_childs (hive_item_t *__dst, hive_item_t *__src)
{
  hive_item_t *node;

  if (!__dst || !__src || !__src->nodes) return -1;
  if (!__dst->nodes)
    __dst->nodes = dyna_create ();

  DYNA_FOREACH (__src->nodes, node);
  dyna_add_to_back (__dst->nodes, node, 0);
  DYNA_DONE;

  return 0;
}

/********
 * Parsing stuff
 */

/**
 * Parsing itration
 *
 * @param __data - buffer to parse
 * @param __token - token will be stored here
 * @param __linum - line number will be stored here
 * @param __flags - flags of token
 * @param __error - pointer to buffer where error's description will be stored
 * @return new shift of data
 */
static char*
hive_parser_iteration (char *__data, char *__token, long *__linenum,
                       int *__flags, char *__error)
{
  int c;
  int len;

  if (!__data)
    {
      return NULL;
    }

  c = len = 0;
  *__token = *__flags = 0;
  strcpy (__error, "");
__main_loop:
  while ((c = *__data) <= ' ')
    {
      if ((c == '\n') && (__linenum))
        {
          (*__linenum)++;
        }

      if (!c)
        {
          return NULL;
        }
      __data++;
    }

  /*  Parse '//' comment */
  if (c == '/' && __data[1] == '/')
    {
      while (*__data && *__data != '\n')
        {
          if (*__data++ == '\n' && __linenum)
            {
              (*__linenum)++;
            }
        }
      goto __main_loop;
    }

  /*  Parse multiline comment */
  if (c == '/' && __data[1] == '*')
    {
      int interior = 1;
      __data++;
      do
        {
          __data++;
          c = *__data;
          if (!c)
            {
              return 0;
            }

          if (c == '\n' && __linenum)
            {
              (*__linenum)++;
            }

          if (c == '/' && __data[1] == '*')
            {
              interior++;
            }

          if (c == '*' && __data[1] == '/')
            {
              interior--;
            }
        }
      while (interior);
      __data += 2;
      goto __main_loop;
    }

  if (c == '[') /* Array */
    {
      int interior = 1;
      c = *(++__data);
      while (interior > 0)
        {
          if (!c)
            {
              strcpy (__error, "Unexpected end of file");
              return __data;
            }

          if (c == '\n' && __linenum)
            {
              (*__linenum)++;
            }

          if (c == '[')
            {
              interior++;
            }

          if (c == ']')
            {
              interior--;
            }

          *(__token + len++) = c;
          c = *(++__data);
        }
      *(__token + len - 1) = 0;
      *__flags |= HPF_ARRAY;
      return __data;
    }

  /* Parse "..." */
  if (c == '\"')
    {
      __data++;
      *__flags |= HPF_STRING;
      for (;;)
        {
          c = *__data++;
          if (c == '\"' || !c)
            {
              *(__token + len) = 0;
              return __data;
            }

          if (c == '\n' && __linenum)
            {
              (*__linenum)++;
            }

          if (c == '\\')
            {
              if (!c) return 0;
              if (c == '\n' && __linenum) (*__linenum)++;

              if (*__data == '\"') *(__token + len) = '\"'; else
              if (*__data == 't') *(__token + len) = '\t'; else
              if (*__data == 'n') *(__token + len) = '\n'; else
                *(__token + len) = *__data;
              __data++;
              c = *__data;
            }
          else *(__token + len) = c;
          len++;
        }
    }

  if (c == '{' || c == '}')
    {
      *(__token + len) = c;
      len++;
      *(__token + len) = 0;
      return __data + 1;
    }

  while (c > 32)
    {
      if (c == '{' || c == '}')
        {
          break;
        }

      if (c == '/' && __data[1] == '*') break; else
      if (c == '/' && __data[1] == '/') break; else
        {
          if (c == '\n')
            {
              if (__linenum)(*__linenum)++;
              break;
            }
        }
      *(__token + len) = c;
      __data++;
      len++;
      c = *__data;
    }
  *(__token + len) = 0;
  return __data;
}

/**
 * Parse buffer
 *
 * @param __data - buffer to parse
 * @param __self - hive tree will be stored here
 * @param __error - pointer to buffer where error's description will be stored
 * @param __cur_dir - current working directory
 * @return zero on success, non-zero otherwise
 */
int
hive_parse_buf (char *__data, dynastruc_t **__self,
                char *__error, char *__cur_dir)
{
  char *shift;
  char token[1024];
  char error[65536];
  int state, flags;
  int name_taked;
  int var_taked;
  int brace_count;
  hive_item_t *current_node;
  char var_name[1024];
  if (!*__self)
    {
      *__self = dyna_create ();
    }
  current_node = hive_spawn_new_node (0, 0, "root", "tree root", 0);
  dyna_push (*__self, current_node, 0);

  shift = __data;
  line = state = name_taked = var_taked = brace_count = 0;

  while ((shift = hive_parser_iteration (shift, token, &line, &flags, error)))
    {
      if (strcmp (error, ""))
        {
          ERROR ("%s at line %ld", error, line);
          return -1;
        }
      if (flags & HPF_ARRAY && state != 1)
        {
          ERROR ("Unexpected array at line %ld", line);
          return -1;
        }
      if (*token == '#' && !(flags & HPF_STRING))
        {
          if (!strcmp (token, "#include"))
            {
              char fn[4096];
              hive_item_t *node;
              dynastruc_t *dummy = 0;
              shift = hive_parser_iteration (shift, token, &line,
                                             &flags, error);

              if (token[0] != '/')
                {
                  snprintf (fn, BUF_SIZE (fn), "%s/%s", __cur_dir, token);
                }
              else
                {
                  strcpy (fn, token);
                }

              hive_parse_file (fn, &dummy, __error);
              node = dyna_data (dyna_head (dummy));
              hive_copy_childs (current_node, node);

              if (node)
                {
                  if (node->header.name)
                    {
                      free (node->header.name);
                    }
                  free (node);
                }
              dyna_destroy (dummy, 0);
            }
          else
            {
              ERROR ("Invalid preprocessor derictive at line %ld", line);
              return -1;
            }
        }
      else
        if (state == 0) /* Name */
        {
          if (*token == '{')
            {
              hive_item_t *temp_node;
              if (!name_taked && !var_taked)
                {
                  ERROR ("Invalid opening bracket at line %ld", line);
                  return -1;
                }
              if (name_taked)
                {
                  hive_add_variable (current_node, var_name, "",
                                     flags | HPF_UNDEFINED);
                }
              temp_node = current_node;
              current_node = temp_node->nodes->tail->data;
              current_node->parent = temp_node;
              brace_count++;
              var_taked = 0;
              name_taked = 0;
            }
          else
            if (*token == '}')
              {
                if (!brace_count || (name_taked && !var_taked))
                  {
                    ERROR ("Invalid closing bracket at line %ld", line);
                    return -1;
                  }
                brace_count--;
                current_node = current_node->parent;
              }
          else
            {
              strcpy (var_name, token);
              name_taked = 1;
              var_taked = 0;

              state = 1;
            }
        }
      else
        if (state == 1) /* Value */
        {
          if (*token == '{' && !(flags & HPF_ARRAY))
            {
              hive_item_t *temp_node;
              if (!name_taked)
                {
                  ERROR ("Invalid opening bracket at line %ld", line);
                  return -1;
                }
              hive_add_variable (current_node, var_name, "", flags);
              temp_node = current_node;
              current_node = current_node->nodes->tail->data;
              current_node->parent = temp_node;
              brace_count++;
              var_taked = name_taked = 0;
            }
          else
            if (*token == '}' && !(flags & HPF_ARRAY))
            {
              if (!brace_count || !var_taked)
                {
                  ERROR ("Invalid closing bracket at line %ld", line);
                  return -1;
                }
              brace_count--;
              current_node = current_node->parent;
            }
          else
            {
              hive_add_variable (current_node, var_name, token, flags);
              name_taked = 0;
              var_taked = 1;
            }
          state = 0;
        }
    }

  if (brace_count)
    {
      ERROR ("No enough closing brackets");
      return -1;
    }
  if (name_taked)
    {
      ERROR ("Variable vithout value at line %ld", line);
      return -1;
    }
  return 0;
}

/**
 * Trim keyname and return index of element in array
 *
 * @param __key - keyname
 * @return index of array's element
 */
static int
get_array_index (char *__key)
{
  int i, len = strlen (__key);
  int lastSlash = 0;
  int index = 0, mult = 1;

  for (i = 0; i < len; i++) if (__key[i] == '/')
    {
      lastSlash = i;
    }

  if (__key[len - 1] != ']' || __key[len - 2] == '\\')
    {
      return -1;
    }

  i = len - 2;
  for (;;)
    {
      if (i <= lastSlash) return -1; // a123]
      if (__key[i] == '[' && __key[i - 1] == '\\') return -1; // a\[123
      if (__key[i] == '[' && lastSlash == i - 1) return -1; // /[123]
      if (__key[i] == '[') break;
      if (__key[i] < '0' || __key[i] > '9') return -1; // a[h]
      index += (__key[i] - '0') * mult;
      mult *= 10;
      i--;
    }
  __key[i] = 0;
  return index;
}

/********
 * End-user stuff
 */

/**
 * Parse file to build hive tree
 *
 * @param __fn - name of file to parse
 * @param __self - hive tree will ebe stored here
 * @param __error - buffer to store error's description
 * @return zero on success, non-zero otherwise
 */
int
hive_parse_file (char *__fn, dynastruc_t **__self, char *__error)
{
  char *data, dir[4096];
  int result;
  data = fload (__fn);
  if (!data)
    {
      strcpy (__error, "Unable to load buffer from file");
      return -1;
    }

  dirname (__fn, dir);

  result = hive_parse_buf (data, __self, __error, dir);
  free (data);
  return result;
}

/**
 * Find item in hive tree
 *
 * @param __self - descriptor of hive tree
 * @param __key - key f item to find
 * @return descriptor of item
 */
dyna_item_t*
hive_find_item (dynastruc_t *__self, char *__key)
{
  dyna_item_t *item;
  char prefix[65535];
  char suffix[65535];
  int i, prefix_taked = 0, prefix_uk = 0, suffix_uk = 0;

  for (i = 0; i < strlen (__key); i++)
    {
      if (__key[i] == '/')
        {
          if (prefix_taked)
            {
              suffix[suffix_uk++] = __key[i];
            }
          prefix_taked = 1;
        }
      else
        {
          if (!prefix_taked)
            {
              prefix[prefix_uk++] = __key[i];
            }
          else
            {
              suffix[suffix_uk++] = __key[i];
            }
        }
    }

  prefix[prefix_uk] = suffix[suffix_uk] = 0;

  dyna_search_reset (__self);
  item = dyna_search (__self, prefix, 0, hive_key_comparator);

  if (!item)
    {
      return NULL;
    }

  if (suffix_uk)
    {
      item = hive_find_item (((hive_item_t*) item->data)->nodes, suffix);
    }

  return item;
}

/**
 * Open hive tree key
 *
 * @param __self - descriptor of hive tree
 * @param __key - key to open
 * @return flexval of key
 */
flex_value_t*
hive_open_key (dynastruc_t *__self, char *__key)
{
  flex_value_t *fv;
  dyna_item_t *item;
  char *dummy;
  int index = -1;
  if (!__self)
    {
      return NULL;
    }
  dummy = malloc (65536);
  sprintf (dummy, "root/%s", __key);

  index = get_array_index (dummy);

  stripslashes (dummy, dummy);

  item = hive_find_item (__self, dummy);

  free (dummy);

  if (!item || !item->data)
    {
      return NULL;
    }

  fv = &(((hive_item_t*) item->data)->header.value);

  if (index >= 0)
    {
      return flexval_get_array_elem (fv, index);
    }

  return fv;
}

/**
 * Free hive tree
 *
 * @param __self - tree to free
 */
void
hive_free_tree (dynastruc_t *__self)
{
  dyna_destroy (__self, hive_dyna_deleter);
}

/**
 * Get hive item by index
 *
 * @param __self - from which item node will be gotten
 * @param __index - index index of node to get
 * @return node descriptor
 */
hive_item_t*
hive_node_by_index (hive_item_t *__self, int __index)
{
  dyna_item_t *item;

  if (!__self)
    {
      return NULL;
    }

  item = dyna_get_item_by_index (__self->nodes, __index);

  if (!item)
    {
      return NULL;
    }

  return item->data;
}

/**
 * Get next sibling node
 *
 * @param __self - base node
 * @return next sibling node
 */
hive_item_t*
hive_next_sibling (hive_item_t *__self)
{
  if (!__self)
    {
      return NULL;
    }

  return hive_node_by_index (__self->parent, __self->index + 1);
}

/**
 * Get first child node
 *
 * @param __self - base node
 * @return first child node
 */
hive_item_t*
hive_first_child (hive_item_t *__self)
{
  dyna_item_t *item;

  if (!__self)
    {
      return NULL;
    }

  item = dyna_head (__self->nodes);

  if (!item)
    {
      return NULL;
    }

  return dyna_data (item);
}

/**
 * Get last child node
 *
 * @param __self - base node
 * @return last child node
 */
hive_item_t*
hive_last_child (hive_item_t *__self)
{
  dyna_item_t *item;

  if (!__self)
    {
      return NULL;
    }

  item = dyna_tail (__self->nodes);

  if (!item)
    {
      return NULL;
    }

  return dyna_data (item);
}

/**
 * Get previous sibling node
 *
 * @param __self - base node
 * @return previous sibling node
 */
hive_item_t*
hive_prev_sibling (hive_item_t *__self)
{
  if (!__self)
    {
      return NULL;
    }
  return hive_node_by_index (__self->parent, __self->index + 1);
}

/********
 * Dumpers
 */

/**
 * Dump hive tree to buffer
 *
 * @param __self - descriptor of hive tree
 * @param __buf - buffer to store dumed tree
 */
void
hive_dump_to_buf (dynastruc_t *__self, char **__buf)
{
  /*
   * TODO: Need to implement this function
   */
}

/**
 * Dump hive tree to file
 *
 * @param __self - hive tree to dump
 * @param __fn - name of file to store dumped tree
 * @return zero on success, non-zero otherwise
 */
int
hive_dump_to_file (dynastruc_t *__self, char *__fn)
{
  /*
   * TODO: Need to implement this function
   */
  return 0;
}
