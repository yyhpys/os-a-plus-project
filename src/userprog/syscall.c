#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "userprog/process.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/synch.h"
#include "filesys/off_t.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "devices/input.h"
#include "lib/kernel/console.h"

/* This is a skeleton system call handler */

static void syscall_handler (struct intr_frame *);
static struct file * fd_to_file(int fd);
static int file_to_fd(struct file * fp);

struct semaphore mutex;

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  sema_init (&mutex,1);
}

static struct file * fd_to_file(int fd)
{
  return thread_current()->u_open_files.file_pointer[fd];
}

static int file_to_fd(struct file * fp)
{
	int i;
	int (*flag)[FDTMAX] = &(thread_current()->u_open_files.open_flag);
	for(i = 3; i < FDTMAX; i++){
		if(*flag[i]==0)
			break;
	}
	
	thread_current()->u_open_files.open_flag[i] = 1;

	thread_current()->u_open_files.file_pointer[i] = fp;
	return i;
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  void **esp_address = &(f->esp);
  uint32_t *eax = &(f->eax);
  int syscall_nr = **((int **)esp_address);
  uint32_t arg1=0,arg2=0,arg3=0;

  switch(syscall_nr) {
    case SYS_EXIT:
	stack_read (&arg1,0,esp_address);
	process_exit_with_status((int)arg1);
	break;
    case SYS_EXEC:
	stack_read (&arg1,0,esp_address);
	*eax = process_execute ((char *)arg1);
	break;
    case SYS_WAIT:
	stack_read (&arg1,0,esp_address);
	*eax = process_wait ((tid_t)arg1);
	break;
	/* hyunjun */
	case SYS_CREATE:
		//uint32_t * arg1 = (uint32_t *) malloc(sizeof(uint32_t));
		//uint32_t * arg2 = (uint32_t *) malloc(sizeof(uint32_t));
		stack_read(&arg1, 0, esp_address);
		stack_read(&arg2, 1, esp_address);
		sema_down (&mutex);
		*eax = filesys_create((char *) arg1, (off_t) arg2);
		sema_up (&mutex);
		break;

	case SYS_OPEN:
		//get arg1 from stack
		//uint32_t * arg1 = (uint32_t *) malloc(sizeof(uint32_t)); 
		stack_read(&arg1, 0, esp_address);
		sema_down (&mutex);
		*eax = file_to_fd(filesys_open((char *) arg1));
		sema_up (&mutex);
		break;

	case SYS_FILESIZE:
		//uint32_t * arg1 = (uint32_t *) malloc(sizeof(uint32_t)); 
		stack_read(&arg1, 0, esp_address);
		sema_down (&mutex);
		*eax = (uint32_t) file_length(fd_to_file((int) arg1));
		sema_up (&mutex);
		break;

	case SYS_READ:
		//get arg1, arg2, arg3 from stack
		//uint32_t * arg1 = (uint32_t *) malloc(sizeof(uint32_t));
		//uint32_t * arg2 = (uint32_t *) malloc(sizeof(uint32_t));
		//uint32_t * arg3 = (uint32_t *) malloc(sizeof(uint32_t));
		stack_read(&arg1, 0, esp_address);
		stack_read(&arg2, 1, esp_address);
		stack_read(&arg3, 2, esp_address);
		
		if(arg1 == 0)
			*eax = (uint32_t) input_getc();
		else
		{
			sema_down (&mutex);
			*eax = file_read(fd_to_file((int)arg1), (void *) arg2, (off_t) arg3);
			sema_up (&mutex);
		}
		break;

	case SYS_WRITE:
		//get arg1, arg2, arg3 from stack
		//uint32_t * arg1 = (uint32_t *) malloc(sizeof(uint32_t));
		//uint32_t * arg2 = (uint32_t *) malloc(sizeof(uint32_t));
		//uint32_t * arg3 = (uint32_t *) malloc(sizeof(uint32_t));
		stack_read(&arg1, 0, esp_address);
		stack_read(&arg2, 1, esp_address);
		stack_read(&arg3, 2, esp_address);
		
		if(arg1 == 1)
			putbuf((const char *) arg2, (size_t) arg3);
		else
		{
			sema_down (&mutex);
			*eax = file_write(fd_to_file((int) arg1), (void *) arg2, (off_t) arg3);
			sema_up (&mutex);
		}
		break;

	case SYS_CLOSE:

		//uint32_t * arg1 = (uint32_t *) malloc(sizeof(uint32_t));
		stack_read(&arg1, 0, esp_address);

		sema_down (&mutex);
		thread_current()->u_open_files.open_flag[arg1] = 0;
		thread_current()->u_open_files.file_pointer[arg1] = NULL;
		file_close(fd_to_file((int)arg1));
		sema_up (&mutex);

		break;
	default:
		break;
  }
}
