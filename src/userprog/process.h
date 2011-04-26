#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"

tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);

/*added*/
void stack_push(char*,int,uint32_t*);
void stack_push_uint(uint32_t,uint32_t*);

#endif /* userprog/process.h  */
