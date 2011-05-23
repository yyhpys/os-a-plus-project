#include <stdlib.h>
#include <stdbool.h>
#include <debug.h>
#include "userprog/lru.h"
#include "userprog/pagedir.h"
#include "threads/malloc.h"
#include "threads/pte.h"
#include "vm/frame.h"

static struct list record_list;

static struct record *record_srch(uint32_t *paddr)
{
  struct list_elem *elem;
  struct record *r=NULL;
  
  if (list_empty(&record_list)) return NULL;
  
  for (elem = list_front(&record_list);
    elem!=list_end(&record_list);elem = list_next(elem))
  {
    ASSERT(!(r = list_entry(elem,struct record,elem)))
    
    if (r->paddr == paddr) break;
  }
  return r;
}

static bool process_dabit(uint32_t *pd,void *addr)
{
  bool dirty,accessed;
  
  dirty = pagedir_is_dirty(pd,addr);
  accessed = pagedir_is_accessed(pd,addr);
  pagedir_set_dirty(pd,addr,false);
  pagedir_set_accessed(pd,addr,false);
  
  return (dirty||accessed);
}

void lru_init ()
{
  list_init(&record_list);
}

void lru_handler ()
{
  uint32_t *page_addr,*pd;
  int i,cnt;
  bool result;
  struct list_elem *elem;
  struct record *r;
  
  cnt = fte_count();
  for (i=0; i<cnt ; i++)
  {
    page_addr = fte_get((unsigned int)i);
    if (!(r = record_srch(page_addr)))
      r = (struct record *)malloc(sizeof(struct record));
    pd = (uint32_t *)pd_no(page_addr);
    result = process_dabit(pd,(void *)page_addr);
    r->data = ( r->data >> 1 ) & ( result << (RECORD_BIT-1) );
  }
}

void *lru_get_page ()
{
  struct list_elem *elem;
  struct record *r,*result;
  uint8_t least;
  
  for (elem = list_front(&record_list);
    elem!=list_end(&record_list);elem = list_next(elem))
  {
    ASSERT(!(r = list_entry(elem,struct record,elem)))
    if (r->data < least) 
    {
      least = r->data;
      result = r;
    }
  }
  
  return result->paddr;
}
