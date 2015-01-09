#include "stdio.h"

#include "arc.h"


void print_raw(uint8_t* data, int length) {
    for(int i = 0; i < length; i++) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

void main() {
    arc_t arc;
    arc_from_string(&arc, "221123123123102122");
    arc_clean(&arc);
    arc_print(&arc);
    arc_add_3gon(&arc);
    arc_print(&arc);    
}
