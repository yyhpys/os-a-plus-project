#include <debug.h>
#include "vm/swap.h"
#include "threads/malloc.h"
#include "devices/disk.h"
#define SWAPMAX 8192

static uint32_t swap_slot_cnt;
static int swap_slot_bitmap[SWAPMAX];

struct disk *swap_disk;

static void *parse_ste(uint32_t *ste)
{
  uint32_t *new = (uint32_t *) malloc(sizeof(uint32_t));
  
  *new = *ste;
  
  new = new & 
void swap_init()
{
	int i;
	list_init(&swap_table);
	swap_slot_cnt = 0;
	for(i = 0; i < SWAPMAX; i++)
		swap_slot_bitmap[i] = 0;
}


int has_empty_slot(uint32_t swap_slot_index)
{
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
}


void swap_in(void *page, uint32_t swap_slot_index)
{
	//ASSERT (has_empty_slot(swap_slot_index)); 
	int i, used_bit;
	struct list_elem *e;
   
	for(e = list_begin(&swap_table); e != list_end(&swap_table); e = list_next(e))
	{
		struct swap_slot *s = list_entry(e, struct swap_slot, elem);
		if(s->index == swap_slot_index){
			list_remove(e);
			swap_slot_bitmap[s->number] = 0;
			used_bit = s->number;
		}
	}
	swap_disk = disk_get(1,1);
	ASSERT(!swap_disk);
   
	for(i = 0; i < 8; i++)
		disk_read(swap_disk, used_bit*8+i, page+DISK_SETOR_SIZE*i);
	swap_slot_cnt++;  
} 

bool swap_out(void *page, uint32_t swap_slot_index)
{
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
	s->index = swap_slot_index;
	for(i = 0; i < 8; i++)
		disk_write(swap_disk, empty*8+i, page+DIST_SECTOR_SIZE*i);
	swap_slot_cnt--;
}
