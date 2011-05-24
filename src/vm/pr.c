#include "vm/pr.h"
#include "userprog/lru.h"
#include "threads/palloc.h"
#include "vm/swap.h"
#include "threads/pte.h"
#include "userprog/process.h"

void page_replacement(void *vaddr)
{
  uint32_t *paddr,*lru_page;
  
  if ((paddr = palloc_get_page(PAL_USER)) != NULL)
    swap_in(vaddr, paddr);
  else
  {
    lru_page = lru_get_page();

    swap_out(lru_page);
    swap_in(vaddr,lru_page);
  }
}

void stack_growth(void *vaddr){
  uint32_t *paddr,*lru_page;
  
  if ((paddr = palloc_get_page(PAL_USER)) != NULL) {
		fte_create(paddr, false);
		install_page_ext (pg_round_down(vaddr), paddr, true);
		set_page_valid(pg_round_down(vaddr), paddr);
	}
  else
  {
    lru_page = lru_get_page();

    swap_out(lru_page);
		set_page_valid(pg_round_down(vaddr), lru_page);
  }

}
