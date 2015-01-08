#ifndef ARC_C
#define ARC_C

#include <stdint.h>

#define NBITS2(n) ((n&2)?1:0)
#define NBITS4(n) ((n&(0xC))?(2+NBITS2(n>>2)):(NBITS2(n)))
#define NBITS8(n) ((n&0xF0)?(4+NBITS4(n>>4)):(NBITS4(n)))
#define NBITS16(n) ((n&0xFF00)?(8+NBITS8(n>>8)):(NBITS8(n)))
#define NBITS32(n) ((n&0xFFFF0000)?(16+NBITS16(n>>16)):(NBITS16(n)))
#define NBITS(n) (n==0?0:NBITS32(n)+1)

#define V 4
#define B (NBITS(4) - 1)
#define NB (64 / B)

#define SIZE 4



typedef union {
    uint64_t d;
    struct {
        uint64_t d0 : B;
        uint64_t d1 : B;
        uint64_t d2 : B;
    };
} arc_component_t;

typedef struct {
    unsigned int length;
    arc_component_t * data;
} arc_t;

void arc_print(arc_t* arc);
void arc_from_string(arc_t* arc, char* str);
void arc_rotate(arc_t* arc);
void arc_normalize(arc_t* arc);
void arc_clean(arc_t* arc);

void arc_add_ngon(arc_t* arc, int ngon);
#endif //ARC_C
