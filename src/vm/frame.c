#include <stdlib.h>
#include <stdbool.h>
#include <debug.h>
#include "threads/palloc.h"
#include "vm/frame.h"
#include "userprog/lru.h"
#include "vm/swap.h"

static struct list fl;

void ft_init(void)
{
  list_init(&fl);
}

void fte_create(void *page_addr, bool user)
{
  struct frame *f;
  struct record *r;
  uint32_t *lru_page;

	if(user)
   {
     f = palloc_get_page(PAL_USER);
     lru_create_record(page_addr);
   }
	else
	  f = palloc_get_page(PAL_ZERO);

	if(f == NULL) {
		if(fte_count()<=0)
			process_exit_with_status(-1);

		lru_page = lru_get_page();

    swap_out(lru_page);
		//set_page_valid(pg_round_down(vaddr), lru_page);
	}

  f->addr = (uint32_t *)page_addr;
  list_push_back(&fl,&f->elem);
}

void fte_destroy(void *page_addr)
{
  struct frame *f;
  int i;
	
	if(fte_count()<=0)
			return;
  if (*page_addr>0xc0000000)
    lru_destroy_record(page_addr);
    
  for(i=0; i<fte_count(); i++)
  {
    f = list_entry(list_pop_front(&fl),struct frame,elem);
    if (f->addr == (uint32_t *) page_addr) break;
    else list_push_back(&fl,&f->elem);
  }
}

int fte_count ()
{
  return list_size(&fl);
}

void *fte_get (unsigned int nth)
{
  unsigned int i = 0;
  struct list_elem *elem;
  struct frame *f;
  
  ASSERT(!list_empty(&fl));
  
  for (elem = list_front(&fl) ; i < nth ; elem = list_next(elem), i++);
  f = list_entry(elem,struct frame,elem);
  
  return f->addr;
}

void *fte_pop ()
{
  uint32_t *addr;
  
  addr = (uint32_t *) fte_get(0);
  fte_destroy(addr);
  
  return addr;
}
