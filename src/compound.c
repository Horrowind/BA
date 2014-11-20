#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "compound.h"

inline int compound_alloc_size() {
    return sizeof(struct compound_node) + valence * sizeof(struct compound_node*);
}

compound_t compound_init_ngon(allocator_t* allocator, int n) {
    compound_t c;
    struct compound_node** ptr = &(c.root);
    for(int i = 1; i < n; i++) {
        (*ptr) = alloc_calloc(allocator);
        ptr = &((*ptr)->nodes[0]);
    }
    (*ptr) = alloc_calloc(allocator);
    (*ptr)->is_end = true;
    
    return c;
}

void compound_print2(struct compound_node* node, int n) {
    for(int i = 0; i < valence; i++) {
        if(node->nodes[i] != NULL) {
            for(int j = 0; j <= n-1; j++) printf(" ");
            printf(">%i\n", i+1);
            compound_print2(node->nodes[i], n+1);
        }
        
    }
}

void compound_print(compound_t* c) {
    compound_print2(c->root, 0);    
}
