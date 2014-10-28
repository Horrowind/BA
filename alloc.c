#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "alloc.h"

allocator_t alloc_init(unsigned int blocksize, unsigned int num_blocks) {
    struct allocator_node* ptr =
        malloc(sizeof(struct allocator_node) + blocksize * num_blocks);
    ptr->next = NULL;
    return (allocator_t) {
        .blocksize  = blocksize,
        .num_blocks = num_blocks,
        .cur_block  = 0,
        .first      = ptr,
        .last       = ptr
    };
    
}

void* alloc_malloc(allocator_t* allocator) {
    printf("allocate!\n");
    struct allocator_node* last = allocator->last;
    if(allocator->cur_block == allocator->num_blocks) {
        struct allocator_node* new =
            malloc(sizeof(struct allocator_node)
                   + allocator->blocksize * allocator->num_blocks);
        new->next = NULL;
        allocator->cur_block = 0;
        last->next = new;
        allocator->last = new;
        return new->data;
    } else {
        allocator->cur_block++;
        return last->data + allocator->cur_block * allocator->blocksize;
    }
}

void* alloc_calloc(allocator_t* allocator) {
    void* ptr = alloc_malloc(allocator);
    memset(ptr, 0, allocator->blocksize);
}

void alloc_free(allocator_t* allocator) {
    struct allocator_node* iter = allocator->first;
    while(iter != NULL) {
        struct allocator_node* tmp = iter->next;
        free(iter);
        iter = tmp;
    }
    allocator->last = NULL;
}
