#ifndef ALLOC_H
#define ALLOC_H

#include <stdint.h>

struct allocator_node {
    struct allocator_node* next;
    uint8_t data[];
};

struct allocator {
    unsigned int blocksize;
    unsigned int num_blocks;
    unsigned int cur_block;
    struct allocator_node* first;
    struct allocator_node* last;
};

typedef struct allocator allocator_t;

allocator_t alloc_init(unsigned int blocksize, unsigned int num_blocks);
void*       alloc_malloc(allocator_t* allocator);
void*       alloc_calloc(allocator_t* allocator);
void        alloc_free(allocator_t* allocator);


#endif //ALLOC_H
