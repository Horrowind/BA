#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "arc.h"

void arc_rotate(arc_t* arc) {
    struct {
        uint8_t d : B;
    } swap;
    swap.d = arc->data[0].d0;
    int i;
    for(i = 0; i < (arc->length / NB); i++) {
        arc->data[i].d >>= B;
        arc->data[i].d |= ((uint64_t)(arc->data[i+1].d0)) << ((NB - 1) * B);
    }
    arc->data[i].d >>= B;
    arc->data[i].d |= ((uint64_t)swap.d) << ((arc->length % NB - 1) * B);
}

uint8_t byte_cmp(uint8_t* data1, uint8_t* data2, unsigned int length) {
    for(int i = length - 1; i >= 0; i--) {
        if(data1[i] > data2[i]) return 1;
        if(data2[i] > data1[i]) return 0;
    }
    return 0;
}
void arc_normalize(arc_t* arc) {
    arc_t tmp_arc;
    int bytelength = (arc->length + NB - 1) / NB * sizeof(uint64_t);
    tmp_arc.length = arc->length;
    tmp_arc.data = malloc(bytelength);
    memcpy(tmp_arc.data, arc->data, bytelength);
    
    for(int i = 0; i < arc->length; i++) {
        arc_rotate(&tmp_arc);
        if(byte_cmp((uint8_t*)tmp_arc.data, (uint8_t*)arc->data, bytelength)) {
            memcpy(arc->data, tmp_arc.data, bytelength);
        }
    }
}

void arc_from_string(arc_t* arc, char* str) {
    arc->length = strlen(str);
    arc->data = malloc((arc->length + NB - 1) / NB * sizeof(uint64_t));
    for(int i = 0; i < arc->length; i++) {
        arc->data[0].d0 = str[i] - '0';
        arc_rotate(arc);
    }
}

void arc_print(arc_t* arc) {
    for(int i = 0; i < arc->length; i++) {
        printf("%i", arc->data[0].d0);
        arc_rotate(arc);
    }
    printf("\n");
}

void arc_clean(arc_t* arc) {
    const int length = arc->length;
    for(int i = 0; i < length; i++) {
        uint8_t d = arc->data[0].d1;
        if(d == 0) {
            arc->length--;
        }
        arc_rotate(arc);
    }
}

void arc_add_ngon(arc_t* arc, int ngon) {
    

}


/* static uint8_t zeros_all[256] = { */
/*     0, 1, 1, 2, 1, 1, 2, 3, 1, 1, 1, 2, 2, 2, 3, 4, 1, 1, 1, 2, 1, 1, 2, 3, 2, 2, 2, 2, 3, 3, 4, 5, 1, 1, 1, 2, 1, 1, 2, 3, 1, 1, 1, 2, 2, */
/*     2, 3, 4, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 4, 4, 5, 6, 1, 1, 1, 2, 1, 1, 2, 3, 1, 1, 1, 2, 2, 2, 3, 4, 1, 1, 1, 2, 1, 1, 2, 3, 2, 2, */
/*     2, 2, 3, 3, 4, 5, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 3, 4, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 6, 7, 1, 1, 1, 2, 1, 1, 2, */
/*     3, 1, 1, 1, 2, 2, 2, 3, 4, 1, 1, 1, 2, 1, 1, 2, 3, 2, 2, 2, 2, 3, 3, 4, 5, 1, 1, 1, 2, 1, 1, 2, 3, 1, 1, 1, 2, 2, 2, 3, 4, 2, 2, 2, 2, */
/*     2, 2, 2, 3, 3, 3, 3, 3, 4, 4, 5, 6, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 3, 4, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 3, 3, 4, 5, 3, */
/*     3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 7, 8 */
/* }; */

/* static uint8_t zeros_left[256] = { */
/*     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, */
/*     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, */
/*     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, */
/*     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, */
/*     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, */
/*     3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 7, 8 */
/* }; */

/* static uint8_t zeros_right[256] = { */
/*     0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, */
/*     1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, */
/*     0, 2, 0, 1, 0, 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 7, 0, 1, 0, 2, 0, 1, 0, */
/*     3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, */
/*     0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5, 0, */
/*     1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 8 */
/* }; */

/* int arc_eval(arc_t* arc, const unsigned int position) { */
/*     unsigned int ones = 0; */
/*     int pos = position; */
/*     int byte = position / 8; */
/*     uint8_t res = pos & 0x7; */
/*     if(res) { */
/*         res = 8 - res; */
/*         ones = zeros_left[((uint8_t*)(arc->data))[byte] & (0xFF >> (8 - res))]; */
/*         if(ones != res) return ones; */
/*         pos -= B; */
/*         byte--; */
/*     } */
/*     while(pos >= 0) { */
/*         res = ((uint8_t*)(arc->data))[byte]; */
/*         ones += zeros_left[res]; */
/*         if(zeros_left[res] != 8) return ones; */
/*         byte--; */
/*         pos -= 8; */
/*     } */
/*     pos = arc->length * B - 1; */
/*     res = pos & 0x7; */
/*     if(res) { */
/*         uint8_t res2 = zeros_left[((uint8_t*)(arc->data))[byte] & (0xFF >> (8 - res))]; */
/*         ones += res2; */
/*         if(res != res2) return ones; */
/*         pos -= B; */
/*         byte--; */
/*     } */
/*     while(pos > position) { */
/*         res = ((uint8_t*)(arc->data))[byte]; */
/*         ones += zeros_left[res]; */
/*         if(zeros_left[res] != 8) return ones; */
/*         byte--; */
/*         pos -= 8; */
/*     } */
/*     return ones; */
/* } */
