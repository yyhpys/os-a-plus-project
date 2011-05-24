#include <list.h>

struct swap_slot
{
	uint32_t number;
	uint32_t index;
	struct list_elem elem;
};

struct list swap_table;

void swap_init(void);
/*int has_empty_slot(uint32_t *);*/
void swap_in(void *, uint32_t *);
void swap_out(void *);

