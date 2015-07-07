#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>

#define VALENCE 3
#define BITS    (VALENCE < 4 ? 1 : 2)
#define MASK    ((1 << BITS) - 1)
#define inttype uint64_t

#define rotl(value, shift, size) (((value << shift) & ((1 << size) - 1)) | (value >> (size - shift)))
#define rotr(value, shift, size) ((value >> shift) | (value << (size - shift)))

struct database_entry {
    inttype boundary;
    //inttype previous;
};

typedef struct {
    struct database_entry* data;
    int length;
    int fill;
} pool_t;

void pool_init(pool_t* pool) {
    pool->data   = malloc(sizeof(struct database_entry));
    pool->length = 1;
    pool->fill   = 0;
}

void pool_alloc(pool_t* pool, inttype boundary) {
    int oldfill = pool->fill;
    pool->fill += 1;
    if(pool->fill > pool->length) {
        pool->length <<= 1;
        pool->data = realloc(pool->data, pool->length * sizeof(struct database_entry));
    }
    pool->data[oldfill].boundary = boundary;
}

void pool_empty(pool_t* pool) {
    pool->fill = 0;
}

void pool_deinit(pool_t* pool) {
    free(pool->data);
}


typedef struct {
    struct database_entry* data;
    int head;
    int tail;
    int length;
} queue_t;

void queue_init(queue_t* queue) {
    queue->head = 0;
    queue->tail = 0;
    queue->length = 2;
    queue->data = malloc(sizeof(struct database_entry)*queue->length);
}

void queue_enqueue(queue_t* queue, inttype boundary) {
    if(queue->head == (queue->tail - 1 + queue->length) % queue->length) {
        struct database_entry* new_data = malloc(sizeof(struct database_entry)*2*queue->length);
        for(int i = 0; i < queue->length; i++) {
            new_data[i] = queue->data[(i + queue->tail) % queue->length];
        }

        //memcpy(new_data, &queue->data[queue->tail], (queue->length - queue->tail)*sizeof(struct database_entry));
        //memcpy(&new_data[queue->length - queue->tail], queue->data, queue->tail*sizeof(struct database_entry));
        queue->tail = 0;
        queue->data[queue->length - 1].boundary = boundary;
        queue->head = queue->length;
        queue->length <<=1;
        free(queue->data);
        queue->data = new_data;
    } else {
        queue->data[queue->head].boundary = boundary;
        queue->head = (queue->head + 1) % queue->length;
    }
}

inttype queue_dequeue(queue_t* queue) {
    //printf("Dequeue: %i\n", (int)queue->data[queue->tail].boundary);
    inttype result = queue->data[queue->tail].boundary;
    queue->tail = (queue->tail + 1) % queue->length;
    return result;
}

uint8_t queue_is_empty(queue_t* queue) {
    return queue->tail == queue->head;
}


void queue_test() {
    srand(time(0));
    queue_t queue;
    queue_init(&queue);
    for(int k = 0; k < queue.length; k++) {
        queue.data[k].boundary = 0;
    }
    int i = 0;
    //printf("Huhu: %i\n", queue.length);
    for(int j = 0; j < 100000000; j++) {
        if((rand() % 2 == 0) && !queue_is_empty(&queue)) {
            queue_dequeue(&queue);
            //printf("Huhu: %i, %i\n", (int)queue_dequeue(&queue), queue.length);
        } else {
            queue_enqueue(&queue, i++);
        }
    }
    printf("Huhu: %i\n", i);

}

const inttype ngon_masks[] =
{
    MASK * ((1 << (BITS *  1)) - 1) / MASK << BITS,
    MASK * ((1 << (BITS *  2)) - 1) / MASK << BITS,
    MASK * ((1 << (BITS *  3)) - 1) / MASK << BITS,
    MASK * ((1 << (BITS *  4)) - 1) / MASK << BITS,
    MASK * ((1 << (BITS *  5)) - 1) / MASK << BITS,
    MASK * ((1 << (BITS *  6)) - 1) / MASK << BITS,
    MASK * ((1 << (BITS *  7)) - 1) / MASK << BITS,
    MASK * ((1 << (BITS *  8)) - 1) / MASK << BITS,
    MASK * ((1 << (BITS *  9)) - 1) / MASK << BITS,
    MASK * ((1 << (BITS * 10)) - 1) / MASK << BITS,
    MASK * ((1 << (BITS * 11)) - 1) / MASK << BITS,
    MASK * ((1 << (BITS * 12)) - 1) / MASK << BITS,
    MASK * ((1 << (BITS * 13)) - 1) / MASK << BITS,
    MASK * ((1 << (BITS * 14)) - 1) / MASK << BITS,
#if (BITS == 1)
    MASK * ((1 << (BITS * 15)) - 1) / MASK << BITS,
    MASK * ((1 << (BITS * 16)) - 1) / MASK << BITS,
    MASK * ((1 << (BITS * 17)) - 1) / MASK << BITS,
    MASK * ((1 << (BITS * 18)) - 1) / MASK << BITS,
    MASK * ((1 << (BITS * 19)) - 1) / MASK << BITS,
    MASK * ((1 << (BITS * 20)) - 1) / MASK << BITS,
    MASK * ((1 << (BITS * 21)) - 1) / MASK << BITS,
    MASK * ((1 << (BITS * 22)) - 1) / MASK << BITS,
    MASK * ((1 << (BITS * 23)) - 1) / MASK << BITS,
    MASK * ((1 << (BITS * 24)) - 1) / MASK << BITS,
    MASK * ((1 << (BITS * 25)) - 1) / MASK << BITS,
    MASK * ((1 << (BITS * 26)) - 1) / MASK << BITS,
    MASK * ((1 << (BITS * 27)) - 1) / MASK << BITS,
    MASK * ((1 << (BITS * 28)) - 1) / MASK << BITS,
    MASK * ((1 << (BITS * 29)) - 1) / MASK << BITS,
    MASK * ((1 << (BITS * 30)) - 1) / MASK << BITS,
#endif
};

const inttype ngon_compare[] =
{
    (VALENCE - 2) * ((1 << (BITS *  1)) - 1) / MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS *  2)) - 1) / MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS *  3)) - 1) / MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS *  4)) - 1) / MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS *  5)) - 1) / MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS *  6)) - 1) / MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS *  7)) - 1) / MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS *  8)) - 1) / MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS *  9)) - 1) / MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS * 10)) - 1) / MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS * 11)) - 1) / MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS * 12)) - 1) / MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS * 13)) - 1) / MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS * 14)) - 1) / MASK << BITS,
#if (BITS == 1)
    (VALENCE - 2) * ((1 << (BITS * 15)) - 1) / MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS * 16)) - 1) / MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS * 17)) - 1) / MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS * 18)) - 1) / MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS * 19)) - 1) / MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS * 20)) - 1) / MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS * 21)) - 1) / MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS * 22)) - 1) / MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS * 23)) - 1) / MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS * 24)) - 1) / MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS * 25)) - 1) / MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS * 26)) - 1) / MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS * 27)) - 1) / MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS * 28)) - 1) / MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS * 29)) - 1) / MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS * 30)) - 1) / MASK << BITS,
#endif
};


inttype normalize(inttype boundary, int size) {
   inttype max = boundary;
    for(int i = 0; i < size; i++) {
        boundary = rotl(boundary, BITS, size);
        if(max < boundary) max = boundary;
    }
    return max;
}

struct boundary_size {
    inttype boundary;
    int size;
};

struct boundary_size insert(inttype boundary, int size, int ngon) {
    struct boundary_size bs = {0, 0};
    unsigned char first = boundary & MASK;
    if(first >= VALENCE - 2) return bs;
    uint32_t shift = __builtin_ctzll(ngon_compare[ngon] ^ (boundary & ngon_masks[ngon]));
    shift = shift - BITS;
    ngon -= 2;
    if(shift / BITS > ngon) return bs;
    ngon -= shift;
    bs.boundary   = ((boundary >> shift) & ~MASK);
    bs.boundary  += (1 << BITS);
    bs.boundary <<= BITS * (ngon);
    bs.boundary  += first + 1;
    bs.size       = size + ngon - shift / BITS;
    //bs.boundary  &= (1 << bs.size) - 1;
    return bs;
}

int small_ngon = 5;
int large_ngon = 7;
#define MAX_SIZE 33
pool_t database[MAX_SIZE];
queue_t queues[MAX_SIZE];

uint8_t database_contains(inttype boundary, int size) {
    for(int i = 0; i < database[size].fill; i++) {
        if(database[size].data[i].boundary == boundary) return 1;
    }
    return 0;
}

void database_add(inttype boundary, int size) {
    //printf("Boundary [%i]: %08x\n", size, (int)boundary);
    pool_alloc(&database[size], boundary);
}


void clever_search(inttype boundary, int size) {
    for(int i = 0; i < MAX_SIZE; i++) {
        pool_init(&database[i]);
        queue_init(&queues[i]);
    }
    queue_enqueue(&queues[size], normalize(boundary, size));
    struct boundary_size bs;
    int sum = 0;

    int found = 0;
    
outer:
    /* for(int i = 3; i < MAX_SIZE; i++) { */
    /*     sum += (queues[i].head - queues[i].tail + queues[i].length) % queues[i].length; */
    /* } */
    //printf("sum: %i\n", sum);
    
    for(int size = 3; size < MAX_SIZE; size++) {
        //printf("%i(%i) ", i, (queues[i].head - queues[i].tail + queues[i].length) % queues[i].length);
        if(!queue_is_empty(&queues[size])) {
            if(found < size) {
                printf("Found %i\n", size);
                found = size;
            }
            //printf("\n");
            boundary = queue_dequeue(&queues[size]);
            if(!database_contains(boundary, size)) {
                database_add(boundary, size);
                for(int j = 0; j < size; j++) {
                    bs = insert(boundary, size, small_ngon);
                    //printf("%04x[%i] ", (int)normalize(bs.boundary, bs.size), bs.size);
                    if(bs.size < MAX_SIZE && bs.size != 0) {
                        /* printf("Huhu:  %08x[%i] (%i)=> %08ux[%u]\n", boundary, size, small_ngon, bs.boundary, bs.size); */
                        /* //if(__builtin_ffsll(normalize(bs.boundary, bs.size)) > bs.size) */
                        /* printf("Huhu!: %08x[%i] (%i)=> %08ux[%u]\n", boundary, size, small_ngon, normalize(bs.boundary, bs.size), bs.size); */
                        queue_enqueue(&queues[bs.size], normalize(bs.boundary, bs.size));
                    }
                    bs = insert(boundary, size, large_ngon);
                    //printf("%04x[%i] ", normalize(bs.boundary, bs.size), bs.size);
                    if(bs.size < MAX_SIZE && bs.size != 0) {
                        /* printf("Huhu:  %08x[%i] (%i)=> %08x[%i]\n", boundary, size, large_ngon, bs.boundary, bs.size); */
                        /* printf("Huhu!: %08x[%i] (%i)=> %08x[%i]\n", boundary, size, large_ngon, normalize(bs.boundary, bs.size), bs.size); */
                        queue_enqueue(&queues[bs.size], normalize(bs.boundary, bs.size));
                    }
                    boundary = rotl(boundary, BITS, size);
                }
            }
            goto outer;
        }
    }
    printf("\n");

}





/* void recursive_search(inttype boundary, int size) { */
/*     struct database_entry* dbe = database_add(boundary, size); */
/*     printf("boundary: %i %016x\n", size, boundary); */
/*     if(dbe->is_done) return; */
/*     struct boundary_size bs; */
/*     for(int i = 0; i < size; i++) { */
/*         bs = insert(boundary, size, small_ngon); */
/*         /\* getchar(); *\/ */
/*         if(bs.size < MAX_SIZE && bs.size != 0) { */
/*             recursive_search(bs.boundary, bs.size); */
/*         } */
/*         bs = insert(boundary, size, small_ngon); */
/*         if(bs.size < MAX_SIZE && bs.size != 0) { */
/*             recursive_search(bs.boundary, bs.size); */
/*         } */
/*         boundary = rotl(boundary, BITS, size); */
/*     } */
/* } */

void insert_test() {
    for(int size = 3; size < 9; size++) {
        for(int i = 0; i < (1 << size); i++) {
            for(int j = 3; j < 4; j++) {
                struct boundary_size bs = insert(i, size, j);
                printf("Insert %i-gon in %02x[%x]: %04x[%x]\n", j, i, size, (int)bs.boundary, bs.size); 
            }
        }
    }
}



int main(int argc, char* argv[]) {
    inttype a = 0;
    if(argc == 3) {
        small_ngon = atoi(argv[1]);
        large_ngon = atoi(argv[2]);
    }
    
    //insert_test();
    //queue_test();
    clever_search(a, 6);
}
