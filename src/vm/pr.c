#include "vm/pr.h"
#include "userprog/lru.h"
#include "threads/palloc.h"
#include "vm/swap.h"

bool page_replacement(void *ste)
{
  uint32_t *paddr,*lru_page;
  
  if (!(paddr = palloc_get_page(PAL_ZERO)))
  {
    lru_page = lru_get_page();
    /*swap*/
    if(!swap_out(lru_page)||!swap_in(lru_page,(uint32_t *)ste))
      return false;
  }
  else if (!swap_in(lru_page,(uint32_t *)ste)) return false;
  
  return true;
}
