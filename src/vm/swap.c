#include "vm/swap.h"
#include "threads/pte.h"
#include "userprog/pagedir.h"

/* swaps in the page from swap disk of index *ste(20bit),
   to the frame address of paddr. returns TRUE if successful
   else FALSE. */
bool swap_in(uint32_t *paddr,uint32_t *ste)
{
  return false;
}

bool swap_out(uint32_t *paddr)
{
  return false;
}
