#include "vm/pr.h"
#include "userprog/lru.h"
#include "threads/palloc.h"
#include "vm/swap.h"

void page_replacement(void *vaddr)
{
  uint32_t *paddr,*lru_page;
  
  if (!(paddr = palloc_get_page(PAL_ZERO)))
    swap_in(vaddr, paddr);
  else
  {
    lru_page = lru_get_page();

    swap_out(lru_page);
    swap_in(vaddr,lru_page);
  }
}
