#include <stdlib.h>
#include <stdbool.h>
#include <debug.h>
#include "threads/palloc.h"
#include "vm/frame.h"

static struct list fl;

void ft_init()
{
  list_init(&fl);
}

void fte_create(void *page_addr)
{
  struct frame *f;
  
  f = palloc_get_page(PAL_ZERO);
  f->addr = (uint32_t *)page_addr;
  list_push_back(&fl,&f->elem);
}

void fte_destroy(void *page_addr)
{
  struct frame *f;
  
  while(1)
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
