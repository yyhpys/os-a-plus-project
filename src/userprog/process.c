#include "userprog/process.h"
#include <debug.h>
#include <inttypes.h>
#include <list.h>
#include <round.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "userprog/gdt.h"
#include "userprog/pagedir.h"
#include "userprog/tss.h"
#include "filesys/directory.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "threads/flags.h"
#include "threads/init.h"
#include "threads/interrupt.h"
#include "threads/palloc.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/malloc.h"
#include "vm/frame.h"
#include "vm/swap.h"

static thread_func start_process NO_RETURN;
static bool load (char *cmdline, void (**eip) (void), void **esp);

int blank = 0, fp = 0;
uint32_t *frame_pointer;
/* Starts a new thread running a user program loaded from
   FILENAME.  The new thread may be scheduled (and may even exit)
   before process_execute() returns.  Returns the new process's
   thread id, or TID_ERROR if the thread cannot be created. */
tid_t
process_execute (const char *file_name) 
{
  char *fn_copy;
  tid_t tid;

  /* Make a copy of "FILE_NAME".
     Otherwise there's a race between the caller and load(). */
  fn_copy = palloc_get_page (0);
  if (fn_copy == NULL)
    return TID_ERROR;
  strlcpy (fn_copy, file_name, PGSIZE);

  /* Create a new thread to execute FILE_NAME. */
  tid = thread_create (file_name, PRI_DEFAULT, start_process, fn_copy);
  if (tid == TID_ERROR)
    palloc_free_page (fn_copy);
  else {
    /*prj3: start*/
    struct thread *current_t = thread_current();
    struct thread *child_t = get_thread_with_tid (tid);

    child_t->parent_t = current_t;
    list_push_back(&(current_t->child_list), &(child_t->childelem));
    /*prj3: end*/
  }

  return tid;
}

/* prj3: reads data *(argv[i]) 
void stack_read_string (char *output,int i, void **esp)
{
	char**** argv_pointer = (char ****) esp;
	int j;
	
	//locating argv 
	*argv_pointer += 8;
	// locating &argv[i] 
	**argv_pointer += 4*i;
	// retrieving argv[i][j] 
	j=0;
	while (1)
	{
		output[j] = ****argv_pointer;
		if ((****argv_pointer)==0) break;
		j++;
		++***argv_pointer;
	}
}
*/

/* prj3: reads data argv[i] */
void stack_read (uint32_t *data,int i,void **esp)
{
	uint32_t *pointer = *((uint32_t **) esp);
	pointer += i+1;
	*data = *pointer;
}

/* prj3: pushes data by 1 byte */
void stack_push (void *data, int bytelength, void **esp)
{
  //printf("\n>>stack_push:start\n");
  int i=bytelength-1;
  char *bytedata = data,*pointer = *((char **)esp);
  while(1)
  {
    if ( i < 0 ) break;
    pointer--;
    //printf("%x=%x,length=%i,i=%i\n",pointer,bytedata[bytelength-i-1],bytelength,i);
    *pointer = bytedata[i];
    i--;
  }  
  *esp = (void *)pointer;
  //printf(">>stack_push:end\n");
}
  

/*prj3: start*/
/*function that returns wait_table from child_tid*/
struct wait_table *
get_wait_table_with_child_tid (tid_t tid)
{ 
  struct list_elem *e;
  struct list *wait_list = waitlist();

  for (e = list_begin (wait_list); e != list_end (wait_list);
       e = list_next (e)) {
    struct wait_table *wtable = list_entry (e, struct wait_table, waitelem);

    if(wtable->child_tid == tid)
			return wtable;
  }

  return NULL;
}
/*prj3: end*/

/* A thread function that loads a user process and starts it
   running. */
static void
start_process (void *file_name_)
{
  char *file_name = file_name_;
  struct intr_frame if_;
  bool success;

  /* Initialize interrupt frame and load executable. */
  memset (&if_, 0, sizeof if_);
  if_.gs = if_.fs = if_.es = if_.ds = if_.ss = SEL_UDSEG;
  if_.cs = SEL_UCSEG;
  if_.eflags = FLAG_IF | FLAG_MBS;

  success = load (file_name, &if_.eip, &if_.esp);

  /* If load failed, quit. */
  palloc_free_page (file_name);
  if (!success) 
    thread_exit ();

  /* Start the user process by simulating a return from an
     interrupt, implemented by intr_exit (in
     threads/intr-stubs.S).  Because intr_exit takes all of its
     arguments on the stack in the form of a `struct intr_frame',
     we just point the stack pointer %esp to our stack frame
     and jump to it. */
  asm volatile ("movl %0, %%esp; jmp intr_exit" : : "g" (&if_) : "memory");
  NOT_REACHED ();
}

/* Waits for thread TID to die and returns its exit status.  If
   it was terminated by the kernel (i.e. killed due to an
   exception), returns -1.  If TID is invalid or if it was not a
   child of the calling process, or if process_wait() has already
   been successfully called for the given TID, returns -1
   immediately, without waiting.

   This function will be implemented in problem 2-2.  For now, it
   does nothing. */
int
process_wait (tid_t child_tid UNUSED) 
{
  enum intr_level old_level;

  struct list *wait_list = waitlist();

  struct thread *current_t = thread_current();
  struct thread *child_t = get_thread_with_tid (child_tid);

  if(current_t->tid != child_t->parent_t->tid)
    return -1;

  struct wait_table *wtable = malloc(sizeof(struct wait_table));
  wtable->self_tid = current_t->tid;
  wtable->child_tid = child_tid;
  wtable->child_status = CHILD_ACTIVE;

  list_push_back(wait_list, &wtable->waitelem);

  for(;;) {

    old_level = intr_disable();
    thread_block();
    intr_set_level(old_level);

    if(wtable->child_status == CHILD_ZOMBIE) {
      int return_status = wtable->return_status;

      list_remove(&wtable->waitelem);
      free(wtable);

      return return_status;
    }

    if(list_size(&(current_t->child_list)) == 0) {
      list_remove(&wtable->waitelem);
      free(wtable);

      return -1;
    }

  }

  return -1;
}

/* prj3: exit with status value */
int
process_exit_with_status (int status) {
  struct thread *current_t = thread_current();
  struct thread *parent_t = current_t->parent_t;
	struct thread *init_t = get_thread_with_tid(1);

  struct list_elem *e;
  struct list *child_list = &current_t->child_list;

  struct wait_table *wtable = get_wait_table_with_child_tid(current_t->tid);

  if(wtable != NULL) {
    wtable->return_status = status;
    wtable->child_status = CHILD_ZOMBIE;

    if(parent_t->tid == wtable->self_tid) {
      thread_unblock(parent_t);
		}


  }
    	for (e = list_begin (child_list); e != list_end (child_list);
       e = list_next (e)) {
    	struct thread *child_thread = list_entry (e, struct thread, childelem);

			child_thread->parent_t = init_t;
			list_push_back(&init_t->child_list, &child_thread->childelem);
		}
  //printf("exit status: %d\n", status);

  thread_exit(); //thread_exit() call process_exit()
}

/* Free the current process's resources. */
void
process_exit (void)
{
  struct thread *cur = thread_current ();
  uint32_t *pd;

	/*prj4:start*/
	struct list *swap_list = &cur->swap_list;
  struct list_elem *e;

	for (e = list_begin (swap_list); e != list_end (swap_list);
       e = list_next (e)) {
    struct swap_slot *s = list_entry (e, struct swap_slot, thread_elem);

		swap_slot_bitmap[s->index] = 0;
  }
	/*prj4:end*/

  /* Destroys the current process's page directory and switch back
     to the kernel-only page directory. */
  pd = cur->pagedir;

	/*prj4*/
	fte_destroy(pd);
	/*prj4*/
  if (pd != NULL) 
    {
      /* Correct ordering here is crucial.  We must set
         cur->pagedir to NULL before switching page directories,
         so that a timer interrupt can't switch back to the
         process page directory.  We must activate the base page
         directory before destroying the process's page
         directory, or our active page directory will be one
         that's been freed (and cleared). */
      cur->pagedir = NULL;
      pagedir_activate (NULL);
      pagedir_destroy (pd);
    }
}

/* Sets up the CPU for running user code in the current
   thread.
   This function is called on every context switch. */
void
process_activate (void)
{
  struct thread *t = thread_current ();

  /* Activate thread's page tables. */
  pagedir_activate (t->pagedir);

  /* Set thread's kernel stack for use in processing
     interrupts. */
  tss_update ();
}

/* We load ELF binaries.  The following definitions are taken
   from the ELF specification, [ELF1], more-or-less verbatim.  */

/* ELF types.  See [ELF1] 1-2. */
typedef uint32_t Elf32_Word, Elf32_Addr, Elf32_Off;
typedef uint16_t Elf32_Half;

/* For use with ELF types in printf(). */
#define PE32Wx PRIx32   /* Print Elf32_Word in hexadecimal. */
#define PE32Ax PRIx32   /* Print Elf32_Addr in hexadecimal. */
#define PE32Ox PRIx32   /* Print Elf32_Off in hexadecimal. */
#define PE32Hx PRIx16   /* Print Elf32_Half in hexadecimal. */

/* Executable header.  See [ELF1] 1-4 to 1-8.
   This appears at the very beginning of an ELF binary. */
struct Elf32_Ehdr
  {
    unsigned char e_ident[16];
    Elf32_Half    e_type;
    Elf32_Half    e_machine;
    Elf32_Word    e_version;
    Elf32_Addr    e_entry;
    Elf32_Off     e_phoff;
    Elf32_Off     e_shoff;
    Elf32_Word    e_flags;
    Elf32_Half    e_ehsize;
    Elf32_Half    e_phentsize;
    Elf32_Half    e_phnum;
    Elf32_Half    e_shentsize;
    Elf32_Half    e_shnum;
    Elf32_Half    e_shstrndx;
  };

/* Program header.  See [ELF1] 2-2 to 2-4.
   There are e_phnum of these, starting at file offset e_phoff
   (see [ELF1] 1-6). */
struct Elf32_Phdr
  {
    Elf32_Word p_type;
    Elf32_Off  p_offset;
    Elf32_Addr p_vaddr;
    Elf32_Addr p_paddr;
    Elf32_Word p_filesz;
    Elf32_Word p_memsz;
    Elf32_Word p_flags;
    Elf32_Word p_align;
  };

/* Values for p_type.  See [ELF1] 2-3. */
#define PT_NULL    0            /* Ignore. */
#define PT_LOAD    1            /* Loadable segment. */
#define PT_DYNAMIC 2            /* Dynamic linking info. */
#define PT_INTERP  3            /* Name of dynamic loader. */
#define PT_NOTE    4            /* Auxiliary info. */
#define PT_SHLIB   5            /* Reserved. */
#define PT_PHDR    6            /* Program header table. */
#define PT_STACK   0x6474e551   /* Stack segment. */

/* Flags for p_flags.  See [ELF3] 2-3 and 2-4. */
#define PF_X 1          /* Executable. */
#define PF_W 2          /* Writable. */
#define PF_R 4          /* Readable. */

static bool setup_stack (void **esp, char *file_name);
static bool validate_segment (const struct Elf32_Phdr *, struct file *);
static bool load_segment (struct file *file, off_t ofs, uint8_t *upage,
                          uint32_t read_bytes, uint32_t zero_bytes,
                          bool writable);

/* Loads an ELF executable from FILE_NAME into the current thread.
   Stores the executable's entry point into *EIP
   and its initial stack pointer into *ESP.
   Returns true if successful, false otherwise. */
bool
load (char *filename, void (**eip) (void), void **esp) 
{
  struct thread *t = thread_current ();
  struct Elf32_Ehdr ehdr;
  struct file *file = NULL;
  off_t file_ofs;
  bool success = false;
  int i;
	char file_name[100];
	
	strtok_n(file_name,filename,' ');

  /* Allocate and activate page directories. */
  t->pagedir = pagedir_create ();
  if (t->pagedir == NULL) 
    goto done;
  process_activate ();

  /* Open executable file. */
  file = filesys_open (file_name);
  if (file == NULL) 
    {
      printf ("load: %s: open failed\n", file_name);
      goto done; 
    }

  /* Read and verify executable header. */
  if (file_read (file, &ehdr, sizeof ehdr) != sizeof ehdr
      || memcmp (ehdr.e_ident, "\177ELF\1\1\1", 7)
      || ehdr.e_type != 2
      || ehdr.e_machine != 3
      || ehdr.e_version != 1
      || ehdr.e_phentsize != sizeof (struct Elf32_Phdr)
      || ehdr.e_phnum > 1024) 
    {
      printf ("load: %s: error loading executable\n", file_name);
      goto done; 
    }

  /* Read program headers. */
  file_ofs = ehdr.e_phoff;
  for (i = 0; i < ehdr.e_phnum; i++) 
    {
      struct Elf32_Phdr phdr;

      if (file_ofs < 0 || file_ofs > file_length (file))
        goto done;
      file_seek (file, file_ofs);

      if (file_read (file, &phdr, sizeof phdr) != sizeof phdr)
        goto done;
      file_ofs += sizeof phdr;
      switch (phdr.p_type) 
        {
        case PT_NULL:
        case PT_NOTE:
        case PT_PHDR:
        case PT_STACK:
        default:
          /* Ignore this segment. */
          break;
        case PT_DYNAMIC:
        case PT_INTERP:
        case PT_SHLIB:
          goto done;
        case PT_LOAD:
          if (validate_segment (&phdr, file)) 
            {
              bool writable = (phdr.p_flags & PF_W) != 0;
              uint32_t file_page = phdr.p_offset & ~PGMASK;
              uint32_t mem_page = phdr.p_vaddr & ~PGMASK;
              uint32_t page_offset = phdr.p_vaddr & PGMASK;
              uint32_t read_bytes, zero_bytes;
              if (phdr.p_filesz > 0)
                {
                  /* Normal segment.
                     Read initial part from disk and zero the rest. */
                  read_bytes = page_offset + phdr.p_filesz;
                  zero_bytes = (ROUND_UP (page_offset + phdr.p_memsz, PGSIZE)
                                - read_bytes);
                }
              else 
                {
                  /* Entirely zero
                     Don't read anything from disk. */
                  read_bytes = 0;
                  zero_bytes = ROUND_UP (page_offset + phdr.p_memsz, PGSIZE);
                }
              if (!load_segment (file, file_page, (void *) mem_page,
                                 read_bytes, zero_bytes, writable))
                goto done;
            }
          else
            goto done;
          break;
        }
    }

  /* Set up stack. */
  if (!setup_stack (esp,filename))
    goto done;

  /* Start address. */
  *eip = (void (*) (void)) ehdr.e_entry;

  success = true;

 done:
  /* We arrive here whether the load is successful or not. */
  file_close (file);
  return success;
}

/* load() helpers. */

static bool install_page (void *upage, void *kpage, bool writable);

/* Checks whether PHDR describes a valid, loadable segment in
   FILE and returns true if so, false otherwise. */
static bool
validate_segment (const struct Elf32_Phdr *phdr, struct file *file) 
{
  /* p_offset and p_vaddr must have the same page offset. */
  if ((phdr->p_offset & PGMASK) != (phdr->p_vaddr & PGMASK)) 
    return false; 

  /* p_offset must point within FILE. */
  if (phdr->p_offset > (Elf32_Off) file_length (file)) 
    return false;

  /* p_memsz must be at least as big as p_filesz. */
  if (phdr->p_memsz < phdr->p_filesz) 
    return false; 

  /* The segment must not be empty. */
  if (phdr->p_memsz == 0)
    return false;
  
  /* The virtual memory region must both start and end within the
     user address space range. */
  if (!is_user_vaddr ((void *) phdr->p_vaddr))
    return false;
  if (!is_user_vaddr ((void *) (phdr->p_vaddr + phdr->p_memsz)))
    return false;

  /* The region cannot "wrap around" across the kernel virtual
     address space. */
  if (phdr->p_vaddr + phdr->p_memsz < phdr->p_vaddr)
    return false;

  /* Disallow mapping page 0.
     Not only is it a bad idea to map page 0, but if we allowed
     it then user code that passed a null pointer to system calls
     could quite likely panic the kernel by way of null pointer
     assertions in memcpy(), etc. */
  //if (phdr->p_vaddr < PGSIZE)
    //return false;

  /* It's okay. */
  return true;
}

/* Loads a segment starting at offset OFS in FILE at address
   UPAGE.  In total, READ_BYTES + ZERO_BYTES bytes of virtual
   memory are initialized, as follows:

        - READ_BYTES bytes at UPAGE must be read from FILE
          starting at offset OFS.

        - ZERO_BYTES bytes at UPAGE + READ_BYTES must be zeroed.

   The pages initialized by this function must be writable by the
   user process if WRITABLE is true, read-only otherwise.

   Return true if successful, false if a memory allocation error
   or disk read error occurs. */
static bool
load_segment (struct file *file, off_t ofs, uint8_t *upage,
              uint32_t read_bytes, uint32_t zero_bytes, bool writable) 
{
  ASSERT ((read_bytes + zero_bytes) % PGSIZE == 0);
  ASSERT (pg_ofs (upage) == 0);
  ASSERT (ofs % PGSIZE == 0);

  file_seek (file, ofs);
  while (read_bytes > 0 || zero_bytes > 0) 
    {
      /* Calculate how to fill this page.
         We will read PAGE_READ_BYTES bytes from FILE
         and zero the final PAGE_ZERO_BYTES bytes. */
      size_t page_read_bytes = read_bytes < PGSIZE ? read_bytes : PGSIZE;
      size_t page_zero_bytes = PGSIZE - page_read_bytes;

      /* Get a page from memory. */
      uint8_t *kpage = palloc_get_page (PAL_USER);
      if (kpage == NULL)
        return false;

			fte_create(kpage, true);

      /* Load this page. */
      if (file_read (file, kpage, page_read_bytes) != (int) page_read_bytes)
        {
          palloc_free_page (kpage);
					fte_destroy(kpage);
          return false; 
        }
      memset (kpage + page_read_bytes, 0, page_zero_bytes);

      /* Add the page to the process's address space. */
      if (!install_page (upage, kpage, writable)) 
        {
          palloc_free_page (kpage);
					fte_destroy(kpage);
          return false; 
        }

      /* Advance. */
      read_bytes -= page_read_bytes;
      zero_bytes -= page_zero_bytes;
      upage += PGSIZE;
    }
  return true;
}

/* Create a minimal stack by mapping a zeroed page at the top of
   user virtual memory. */
static bool
setup_stack (void **esp, char *file_name) 
{
  uint8_t *kpage;
  bool success = false;

  uint32_t tokaddr[100],temp;
  char *ptr,*str,*token,blank[4];
  int size,j,i=0;

  kpage = palloc_get_page (PAL_USER | PAL_ZERO);
  if (kpage != NULL) 
    {
      success = install_page (((uint8_t *) PHYS_BASE) - PGSIZE, kpage, true);
      if (success)
	{
        *esp = PHYS_BASE;
	/* argument stacking */
  /* start */

  str = file_name;
  while(1)
  {
    token=strtok_r(str," ",&ptr);
    if(token[0]<=4) break;
    stack_push((void *)token,(strlen(token)+1),esp);
    tokaddr[i]=(uint32_t)*esp;
    
    i++;
    str=NULL;
  }
  size=i;

  /* pushing blank */
  for (j=0;j<4;j++) blank[j]=0;
  stack_push((void *)blank,(int)(((uint32_t)(*esp))%4),esp);
  /* pushing argv[j]=0 */
  stack_push((void *)blank,4,esp);

  /* pushing rest */
  while(1)
  {
    stack_push((void *)&tokaddr[i-1],4,esp);
    if (i==1)break;
    i--;
  }
  temp = (uint32_t)*esp;
  stack_push((void *)&temp,4,esp);
  stack_push((void *)&size,4,esp);
  stack_push((void *)blank,4,esp);
  /*end*/
		}
      else
        palloc_free_page (kpage);
    }
  return success;
}

/* Adds a mapping from user virtual address UPAGE to kernel
   virtual address KPAGE to the page table.
   If WRITABLE is true, the user process may modify the page;
   otherwise it is read-only.
   UPAGE must not already be mapped.
   KPAGE should probably be a page obtained from the user pool
   with palloc_get_page().
   Returns true on success, false if UPAGE is already mapped or
   if memory allocation fails. */
static bool
install_page (void *upage, void *kpage, bool writable)
{
  struct thread *t = thread_current ();

  /* Verify that there's not already a page at that virtual
     address, then map our page there. */
  return (pagedir_get_page (t->pagedir, upage) == NULL
          && pagedir_set_page (t->pagedir, upage, kpage, writable));
}

bool
install_page_ext (void *upage, void *kpage, bool writable)
{
		return install_page(upage, kpage, writable);
}
