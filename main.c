#include "alloc.h"
#include "compound.h"

void main() {
    allocator_t a = alloc_init(compound_alloc_size(), 0x400);
    compound_t c = compound_init_ngon(&a, 5);
    compound_print(&c);
    alloc_free(&a);
}
