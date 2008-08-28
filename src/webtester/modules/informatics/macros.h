/*
 *
 * ================================================================================
 *  macros.h - part of the WebTester Server
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#ifndef _wt_informatics_macros_h_
#define _wt_informatics_macros_h_

#define INF_MESSAGE(__proc, __text, __args...) __proc ("Informatics", __text, ##__args)
#define INF_INFO(__text, __args...)     INF_MESSAGE (MODULE_INFO,     __text, ##__args)
#define INF_WARNING(__text, __args...)  INF_MESSAGE (MODULE_WARNING,  __text, ##__args)
#define INF_ERROR(__text, __args...)    INF_MESSAGE (MODULE_ERROR,    __text, ##__args)
#define INF_DEBUG(__text, __args...)    INF_MESSAGE (MODULE_DEBUG,    __text, ##__args)

#define INF_KEY_ENTRY(__proc, __val, __path)  \
  __proc ("Informatics", __val, __path)

#define INF_INT_KEY(__val, __path)   INF_KEY_ENTRY (MODULE_INT_KEY,   __val, __path)
#define INF_FLOAT_KEY(__val, __path) INF_KEY_ENTRY (MODULE_FLOAT_KEY, __val, __path)
#define INF_PCHAR_KEY(__val, __path) INF_KEY_ENTRY (MODULE_PCHAR_KEY, __val, __path)

#define INF_SAFE_INT_KEY(__val, __path, __default) \
  { \
    __val=__default; \
    INF_INT_KEY (__val, __path); \
  }
#define INF_SAFE_FLOAT_KEY(__val, __path, __default) \
  { \
    __val=__default; \
    INF_FLOAT_KEY (__val, __path); \
  }
#define INF_SAFE_PCHAR_KEY(__val, __path, __default) \
  { \
    strcpy (__val, __default); \
    INF_PCHAR_KEY (__val, __path); \
  }

// Runtime error in executing process
#define PROCESS_RUNTIME_ERROR(__self) \
  (RUN_PROC_TERMINATED (__self) || \
   (RUN_PROC_FINISHED (__self) && RUN_PROC_EXITCODE(__self)))

#define SAFE_FREE_PROC(__self) \
  { \
    if (__self) \
      { \
        run_free_process (__self); \
        __self=0; \
      } \
  }

////
// Compiler's config reading stuff

#define COMPILER_KEY(__id,__path,__func) \
  __func (Informatics_compiler_config_val (__id, __path))
#define COMPILER_INT_KEY(__id,__path) \
  COMPILER_KEY (__id, __path, flexval_get_int)
#define COMPILER_FLOAT_KEY(__id,__path) \
  COMPILER_KEY (__id, __path, flexval_get_float)
#define COMPILER_PCHAR_KEY(__id,__path) \
  COMPILER_KEY (__id, __path, flexval_get_string)

#define COMPILER_SAFE_VAL(__id,__path,__default_value,__func) \
  ((Informatics_compiler_config_val (__id, __path))?(__func (Informatics_compiler_config_val (__id, __path))):(__default_value))
#define COMPILER_SAFE_INT_KEY(__id,__path,__default_value) \
  COMPILER_SAFE_VAL (__id, __path, __default_value, flexval_get_int)
#define COMPILER_SAFE_FLOAT_KEY(__id,__path,__default_value) \
  COMPILER_SAFE_VAL (__id, __path, __default_value, flexval_get_float)
#define COMPILER_SAFE_PCHAR_KEY(__id,__path,__default_value) \
  COMPILER_SAFE_VAL (__id, __path, __default_value, flexval_get_string)

#define COMPILER_SAFE_COMMON_VAL(__path,__default_value,__func) \
  ((Informatics_compiler_common_val (__path))?(__func (Informatics_compiler_common_val (__path))):(__default_value))
#define COMPILER_SAFE_COMMON_INT_KEY(__path,__default_value) \
  COMPILER_SAFE_COMMON_VAL (__path, __default_value, flexval_get_int)
#define COMPILER_SAFE_COMMON_FLOAT_KEY(__path,__default_value) \
  COMPILER_SAFE_COMMON_VAL (__path, __default_value, flexval_get_float)
#define COMPILER_SAFE_COMMON_PCHAR_KEY(__path,__default_value) \
  COMPILER_SAFE_COMMON_VAL (__path, __default_value, flexval_get_string)

#define TASK_COMPILER_ID(__self) \
  (char*)TASK_INPUT_PARAM (__self, "COMPILERID")

#define REPLACE_VAR(__str, __var, __val) \
  preg_replace ("/\\$\\{" __var "\\}/gs", __val, __str);

#define INF_LOG(__text,__args...)\
  LOG ("informatics", __text, ##__args)

#define INF_DEBUG_LOG(__text,__args...)\
  DEBUG_LOG ("informatics", __text, ##__args)

#endif
