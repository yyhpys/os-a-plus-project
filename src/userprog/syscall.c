#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "userprog/process.h"
#include "threads/interrupt.h"
#include "threads/thread.h"

/* This is a skeleton system call handler */

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  printf ("system call!\n");

  void **esp_address = &(f->esp);

  

  int syscall_nr;
  stack_read (&syscall_nr,-1,esp_address);

  int int_arg;
  char *char_arg;
  tid_t tid_arg;

  switch(syscall_nr) {
    case SYS_EXIT:
	stack_read (&int_arg,0,esp_address);
	process_exit_with_status(int_arg);
	break;
    case SYS_EXEC:
	stack_read (&char_arg,0,esp_address);
	f->eax = process_execute (char_arg);
	break;
    case SYS_WAIT:
	stack_read (&tid_arg,0,esp_address);
	f->eax = process_wait (tid_arg);
	break;
    case SYS_CREATE:
    case SYS_REMOVE:
    case SYS_OPEN:
    case SYS_FILESIZE:
    case SYS_READ:
    case SYS_WRITE:
	break;
  }
}
