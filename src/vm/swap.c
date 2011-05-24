#include <debug.h>
#include <stdbool.h>
#include "vm/swap.h"
#include "threads/malloc.h"
#include "threads/pte.h"
#include "devices/disk.h"
#include "userprog/pagedir.h"
#define SWAPMAX 8192

static uint32_t swap_slot_cnt;
static int swap_slot_bitmap[SWAPMAX];

struct disk *swap_disk;

static uint32_t parse_pte(void *vaddr)
{
  uint32_t new, *ste = pagedir_get_pte(vaddr);
  new = *ste;
  
  new = (new & 0xfffff000)>>12;
  return new;
} 

void swap_init()
{
	int i;
	list_init(&swap_table);
	swap_slot_cnt = 0;
	for(i = 0; i < SWAPMAX; i++)
		swap_slot_bitmap[i] = 0;
}

/*
int has_empty_slot(uint32_t *pte)
{
  uint32_t swap_slot_index;
	int retval = 0;
	struct list_elem *e;
	
	if(swap_slot_cnt >= swap_slot_index && swap_slot_cnt < SWAPMAX)
		retval = 1;
	else{
		for(e = list_begin(&swap_table); e != list_end(&swap_table); e = list_next(e)){
			struct swap_slot *s = list_entry(e, struct swap_slot, elem);
			if(s->index == swap_slot_index)
				retval = 1;
		}			
	}
	return retval;
}*/
void set_page_valid (void *vaddr, uint32_t *paddr)
{
  *paddr = *paddr >> 12;
  pagedir_set_valid((uint32_t *)pd_no(vaddr),vaddr,1);
  pagedir_set_paddr((uint32_t *)pd_no(vaddr),vaddr,*paddr);
}

static void set_page_invalid (void *vaddr, uint32_t index)
{
  pagedir_set_valid((uint32_t *)pd_no(vaddr),vaddr,0);
  pagedir_set_paddr((uint32_t *)pd_no(vaddr),vaddr,index);
}

int swap_count ()
{
		  return list_size(&swap_table);
}

bool is_page_exist(void *vaddr)
{
  uint32_t swap_slot_index;
	int i;
	struct swap_slot *s;
  
  swap_slot_index = parse_pte(vaddr);
  ASSERT(swap_slot_index < SWAPMAX);
  
	for(i=0; i<swap_count(); i++)
	{
		s = list_entry(list_pop_front(&swap_table), struct swap_slot, elem);
		list_push_back(&swap_table,&s->elem);
		if(s->number == swap_slot_index){
			return true;
		}	
	}

	return false;
}

void swap_in(void *vaddr, uint32_t *page)
{
	//ASSERT (has_empty_slot(swap_slot_index)); 
	uint32_t swap_slot_index;
	int i, used_bit = 0;
	struct swap_slot *s;
  
  swap_slot_index = parse_pte(vaddr);
  ASSERT(swap_slot_index < SWAPMAX);
  
	while(1)
	{
		s = list_entry(list_pop_front(&swap_table), struct swap_slot, elem);
		if(s->number == swap_slot_index){
			swap_slot_bitmap[s->number] = 0;
			used_bit = s->number;
			break;
		}
		list_push_back(&swap_table,&s->elem);
	}
	swap_disk = disk_get(1,1);
	ASSERT(!swap_disk);
   
	for(i = 0; i < 8; i++)
		disk_read(swap_disk, used_bit*8+i, page+DISK_SECTOR_SIZE*i);
	swap_slot_cnt++;
	
	set_page_valid(vaddr, page);
} 

void swap_out(void *vaddr)
{
  uint32_t *page = pagedir_get_page ((uint32_t *)pd_no(vaddr),vaddr);
	int i, empty;
	struct swap_slot *s  = malloc(sizeof(struct swap_slot));
	swap_disk = disk_get(1,1);
  ASSERT(!swap_disk);
  
	for(empty = 0; empty < SWAPMAX; empty++){
		if (swap_slot_bitmap[empty] == 0)
			break;
	}
	swap_slot_bitmap[empty] = 1;
	list_push_front(&swap_table, &(s->elem));
	s->number = empty;
	for(i = 0; i < 8; i++)
		disk_write(swap_disk, empty*8+i, page+DISK_SECTOR_SIZE*i);
	swap_slot_cnt--;
	
	set_page_invalid (vaddr, s->number);
}
