#include <stdlib.h>
#include <list.h>

#define RECORD_BIT 8

struct record
  {
    uint8_t data;
    uint32_t *paddr;
    struct list_elem elem;
  };

void lru_init(void);
void lru_handler(void);
void *lru_get_page(void);
void lru_create_record(void *);
void lru_destroy_record(void *);
