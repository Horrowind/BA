
#ifndef COMPOUND_H
#define COMPOUND_H

#include <stdbool.h>
#include "alloc.h"

static int valence = 4;

struct compound_node {
    bool is_end;
    struct compound_node* nodes[];
};

struct compound_list {
    struct compound_node* node;
    struct compound_list* next;
};

struct compound {
    allocator_t* allocator;
    struct compound_node* root;
};
typedef struct compound compound_t;




compound_t  compound_init_ngon(allocator_t* allocator, int n);
compound_t* compound_merge(compound_t*, compound_t*);
compound_t* compound_add(compound_t*, compound_t*);
void        compound_rotate(compound_t*);
compound_t* compound_copy(compound_t*);
void        compound_print(compound_t*);
int         compound_alloc_size();

#endif //COMPOUND_H
