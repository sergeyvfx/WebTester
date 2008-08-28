#include <libwebtester/core.h>
#include <libwebtester/conf.h>
#include <libwebtester/network-soup.h>

#include "../webtester/autoinc.h"
#include "../webtester/const.h"

#include <dlfcn.h>
#include <librun/run.h>


/*typedef int  (*run_init_proc) (void);
typedef void (*run_done_proc) (void);
typedef run_process_info_t* (*run_create_process_proc) (char *, char *, DWORD, DWORD);
typedef void (*run_free_process_proc) (run_process_info_t*);
typedef void (*run_execute_process_proc) (run_process_info_t*);
typedef void (*run_pwait_proc) (run_process_info_t*);*/

int
main                               (int __argc, char **__argv)
{

  char *dir;
  dynastruc_t *dyna=dir_listing ("./tmp");
  DYNA_FOREACH (dyna, dir);
  printf ("%s\n", dir);

  DYNA_DONE ();
  
  dyna_destroy (dyna, dyna_deleter_free_ref_data);

/*  void *handle=dlopen ("./librun", RTLD_LAZY);
  run_init_proc run_init;
  run_done_proc run_done;
  run_create_process_proc run_create_process;
  run_free_process_proc run_free_process;
  run_execute_process_proc run_execute_process;
  run_pwait_proc run_pwait;

  core_init ();
  config_init (CONFIG_FILE);
  
  core_enter_debug_mode ();
  core_print (MSG_INFO, "");
  core_set_last_error ("");

  if (!handle)
    {
       printf ("Error loading librun (%s)\n", dlerror ());
       return -1;
    }

  run_init=dlsym (handle, "run_init");
  run_done=dlsym (handle, "run_done");
  run_create_process=dlsym (handle, "run_create_process");
  run_free_process=dlsym (handle, "run_free_process");
  run_execute_process=dlsym (handle, "run_execute_process");
  run_pwait=dlsym (handle, "run_pwait");

  if (!run_init ())
    {
      run_process_info_t *proc=run_create_process ("./a.out", "./", -1, -1);

      if (proc)
        {
/*          int i;
          for (i=0; i<100; i++)
            {
              printf ("############");
              printf ("## Iteration #%d\n", i);
              run_execute_process (proc);
              run_pwait (proc);
              //if (proc->state!=4) printf ("Big troubles!! %d\n", proc->state);
              printf ("STATE: %d\n", proc->state);
              run_free_process (proc);
              proc=run_create_process ("./a.out", "./", -1, 500000);
            }*/

/*          run_execute_process (proc);
          run_pwait (proc);
          if (RUN_PROC_STATE (*proc)&PS_FINISHED && !RUN_PROC_EXITCODE (*proc))
            {
              if (proc->state==PS_FINISHED)
                {
                  if (RUN_PROC_PIPEBUF_LEN (*proc)>0)
                    printf ("Buffer from pipe:\n%s\n", RUN_PROC_PIPEBUF (*proc));

                  printf ("Time usage: %lldusec\n",  RUN_PROC_TIMEUSAGE (*proc));
                  printf ("Memory usage: %lldKb\n",  RUN_PROC_MEMUSAGE (*proc));
                  printf ("Exit code: %d\n",         RUN_PROC_EXITCODE (*proc));
                } else
                {
                  if (RUN_PROC_MEMORYLIMIT (*proc))
                    printf ("Memory limit\n");
                  if (RUN_PROC_MEMORYLIMIT (*proc))
                    printf ("Time limit\n");
                }
            } else
            {
              if (RUN_PROCESS_RUNTIME_ERROR (*proc))
                printf ("Runtime error\n"); else
              {
                if (RUN_PROC_MEMORYLIMIT (*proc))
                  printf ("Memory limit\n");
                if (RUN_PROC_MEMORYLIMIT (*proc))
                  printf ("Time limit\n");
              }
            }
            run_free_process (proc);
        }
      run_done ();
    } else
      printf ("Error initializing RUN\n");

  dlclose (handle);

  return 0;

/*  struct timespec timestruc;
  char ptr[1024];
  int i;
  
  http_session_t *session;
  http_message_t *msg;

  g_thread_init (NULL);
  g_type_init ();

  regexp_init ();
  core_init ();
  config_init (CONFIG_FILE);

  session=http_session_new (FALSE);

  timestruc.tv_sec=0;
  timestruc.tv_nsec=0.2*1000*1000; // Nanoseconds :)

  for (;;)
    {
      for (i=0; i<100; i++) {
      msg=soup_message_new ("GET", "http://127.0.0.1/wiki");
      soup_session_send_message (session, msg);
      g_object_unref (msg);
      }
    }

//  CONFIG_PCHAR_KEY (ptr, "Server/Modules/Informatics/RetProps[1]");
//  printf ("%s\n", ptr);

  config_done ();
  return 0;*/
}
