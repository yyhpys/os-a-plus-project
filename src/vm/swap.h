#include <list.h>
#define SWAPMAX 8192

struct swap_slot
{
	uint32_t number;
	uint32_t index;
	struct list_elem elem;
	struct list_elem thread_elem;
};

struct list swap_table;
static int swap_slot_bitmap[SWAPMAX];

void swap_init(void);
/*int has_empty_slot(uint32_t *);*/
void swap_in(void *, uint32_t *);
void swap_out(void *);

void set_page_valid (void *vaddr, uint32_t *paddr);

bool is_page_exist(void *);
