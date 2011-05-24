#ifndef USERPROG_PAGEDIR_H
#define USERPROG_PAGEDIR_H

#include <stdbool.h>
#include <stdint.h>

uint32_t *pagedir_create (void);
void pagedir_destroy (uint32_t *pd);
bool pagedir_set_page (uint32_t *pd, void *upage, void *kpage, bool rw);
void *pagedir_get_page (uint32_t *pd, const void *upage);
void pagedir_clear_page (uint32_t *pd, void *upage);
bool pagedir_is_dirty (uint32_t *pd, const void *upage);
void pagedir_set_dirty (uint32_t *pd, const void *upage, bool dirty);
void pagedir_set_valid (uint32_t *pd, const void *upage, bool valid);
void pagedir_set_paddr (uint32_t *pd, const void *vpage, uint32_t src);
bool pagedir_is_accessed (uint32_t *pd, const void *upage);
void pagedir_set_accessed (uint32_t *pd, const void *upage, bool accessed);
void pagedir_activate (uint32_t *pd);

uint32_t *pagedir_get_pte (void *vaddr);

uint32_t *lookup_page_ext (uint32_t *pd, const void *vaddr, bool create);

#endif /* userprog/pagedir.h  */
