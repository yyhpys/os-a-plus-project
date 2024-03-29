#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"

tid_t process_execute (const char *file_name);
int process_wait (tid_t);
/*prj3: start*/
int process_exit_with_status (int status);
/*prj3: end*/
void process_exit (void);
void process_activate (void);

/*prj3: start*/
void stack_read (uint32_t *,int,void **);
void stack_push (void *,int,void **);

struct wait_table *get_wait_table_with_child_tid (tid_t);

enum wait_status
  {
    CHILD_ACTIVE,
    CHILD_ZOMBIE,
  };

struct wait_table
  {
    tid_t self_tid;
    tid_t child_tid;
    enum wait_status child_status; 
    int return_status;

    struct list_elem waitelem;
  };
/*prj3: end*/
bool
install_page_ext (void *upage, void *kpage, bool writable);
#endif /* userprog/process.h  */
