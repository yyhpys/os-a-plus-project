#include <list.h>

struct frame
  {
    uint32_t *addr;
    struct list_elem elem;
  };

void ft_init (void);
void fte_create (void *, bool);
void fte_destroy (void *);
int fte_count (void);
void *fte_get (unsigned int);
void *fte_pop (void);
