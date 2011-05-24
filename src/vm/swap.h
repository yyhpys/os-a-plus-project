#include <list.h>

struct swap_slot
{
	uint32_t number;
	uint32_t index;
	struct list_elem elem;
};

struct list swap_table;

void swap_init();
int has_empty_slot(uint32_t swap_slot_index);
void swap_in(void *page, uint32_t swap_slot_index);
void swap_out(void *page, uint32_t swap_slot_index);

