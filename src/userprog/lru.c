#include <stdlib.h>
#include <stdbool.h>
#include <debug.h>
#include "userprog/lru.h"
#include "userprog/pagedir.h"
#include "threads/malloc.h"
#include "threads/pte.h"
#include "vm/frame.h"
#include "threads/palloc.h"

static struct list record_list;

static struct record *record_srch(uint32_t *paddr)
{
  struct list_elem *e;
  struct record *r=NULL;
  
  if (list_empty(&record_list)) return NULL;
  
  for (e = list_front(&record_list);
    e!=list_end(&record_list);e = list_next(e))
  {
    if((r = list_entry(e,struct record,elem)) == NULL)
				return NULL;
    
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
  struct record *r;
  
  cnt = fte_count();
  for (i=0; i<cnt ; i++)
  {
    page_addr = fte_get((unsigned int)i);
    r = record_srch(page_addr);
    ASSERT(!r);
    pd = (uint32_t *)pd_no(page_addr);
    result = process_dabit(pd,(void *)page_addr);
    r->data = ( r->data >> 1 ) & ( result << (RECORD_BIT-1) );
  }
}

void *lru_get_page ()
{
  struct list_elem *e;
  struct record *r,*result;
  uint8_t least=0xff;
  
  for (e = list_front(&record_list);
    e!=list_end(&record_list);e = list_next(e))
  {
    ASSERT(!(r = list_entry(e,struct record,elem)))
    if (r->data < least) 
    {
      least = r->data;
      result = r;
    }
  }
  
  return result->paddr;
}

void lru_create_record(void *paddr)
{
	struct record *r;
  r = (struct record *)malloc(sizeof(struct record));
  list_push_back(&record_list, &r->elem);
  r->data = 0x00;
  r->paddr = (uint32_t *)paddr;
}

void lru_destroy_record(void *paddr)
{
  struct record *r;
  r = record_srch(paddr);
 
	if(r != NULL){
  	list_remove(&r->elem);
  	free(r);
	}
}
