#include <assert.h>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include <immintrin.h>


#define VALENCE     3
#define BITS        (VALENCE < 4 ? 1 : 2)
#define MASK        ((1ull << BITS) - 1)
#define inttype     uint64_t
#define NUM_THREADS 5

#define rotl(value, shift, size) (((value << shift) & ((1ull << size) - 1)) | (value >> (size - shift)))
#define rotr(value, shift, size) (((value >> shift) | (value << (size - shift))) & ((1ull << size) - 1))

uint32_t hash_prime_table[] = {
    2,
    2,
    2,
    5,
    11,
    19,
    41,
    79,
    157,
    317,
    631,
    1259,
    2531,
    5059,
    10133,
    20249,
    40507,
    81001,
    162011,
    324031,
    648059,
    1296109,
    2592221,
    5184433,
    10368881,
    20737781,
    41475583,
    82951123,
    165902239,
    331804481,
    663608929,
    1327217867,
    2654435761,
};

typedef struct {
    inttype* data;
    int length;
    int fill;
} hashmap_t;

uint32_t hash(inttype input, int size) {
    return input * hash_prime_table[size] % (1ull << size);
}

void hash_init(hashmap_t* pool) {
    pool->data   = malloc(sizeof(inttype));
    pool->length = 8;
    pool->fill   = 0;
}

void hash_insert(hashmap_t* pool, inttype input) {
    
}

typedef struct {
    inttype* data;
    int head;
    int tail;
    int length;
} queue_t;

void queue_init(queue_t* queue) {
    queue->head = 0;
    queue->tail = 0;
    queue->length = 2;
    queue->data = malloc(sizeof(inttype)*queue->length);
    for(int k = 0; k < queue->length; k++) {
        queue->data[k] = 0;
    }
}

void queue_deinit(queue_t* queue) {
    queue->length = 0;
    free(queue->data);
}

void queue_enqueue(queue_t* queue, inttype boundary) {
    if(queue->head == (queue->tail - 1 + queue->length) % queue->length) {
        queue->data[queue->head] = boundary;
        inttype* new_data = malloc(sizeof(inttype)*2*queue->length);
        memset(new_data, 0, sizeof(inttype)*2*queue->length);
        for(int i = 0; i < queue->length; i++) {
            new_data[i] = queue->data[(i + queue->tail) % queue->length];
        }

        queue->tail = 0;
        queue->data[queue->length - 1] = boundary;
        queue->head = queue->length;
        queue->length *= 2;
        free(queue->data);
        queue->data = new_data;
    } else {
        queue->data[queue->head] = boundary;
        queue->head = (queue->head + 1) % queue->length;
    }
}

inttype queue_dequeue(queue_t* queue) {
    //printf("Dequeue: %i\n", (int)queue->data[queue->tail]);
    inttype result = queue->data[queue->tail];
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

    
    //printf("Huhu: %i\n", queue.length);
    for(int i = 0; i < 100000;) {
        if((rand() % 3 != 0) && !queue_is_empty(&queue)) {
            printf("Huhu: %i, %i\n", (int)queue_dequeue(&queue), queue.length);
        } else {
            queue_enqueue(&queue, i++);
        }
    }
    getchar();
    while(!queue_is_empty(&queue)) {
         printf("Huhu: %i, %i\n", (int)queue_dequeue(&queue), queue.length);
    }
}

void queue_test2() {
    queue_t queue;
    queue_init(&queue);
    for(int j = 0; ; j++) {
        for(int i = 0; i < queue.length; i++) {
            printf("%lx ", queue.data[i]);
        }
        getchar();
        queue_enqueue(&queue, j);
    }
}

inline void check_boundary_size(inttype boundary, int size) {
#   ifndef NDEBUG
    if(boundary != 0 && sizeof(inttype) * 8 - __builtin_clzll(boundary) > size * BITS) {
        printf("Boundary %lx is larger then size %i\n", boundary, size);
        exit(0);
    }
#   endif
}

inline inttype ngon_masks(int size) {
    assert(size <= (BITS - 1 ? 14 : 31));
    return (MASK << (BITS *  (size - 1)));
}

inline inttype ngon_add_compare(int size) {
    assert(size <= (BITS - 1 ? 14 : 31));
    return (VALENCE - 2) * (((1ull << (BITS *  (size - 1))) - 1) / MASK);
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

struct boundary_size insert_ngon(inttype boundary, int size, int ngon) {
    struct boundary_size bs = {.boundary = 0, .size = 0};
    // Check if we can add an edge to the first node)
    if(boundary >> (BITS * (size - 1)) < VALENCE - 2) {
        bs.boundary = boundary + (1 << (BITS * (size - 1)));
        uint32_t shift = __builtin_ctzll(ngon_add_compare(size) ^ bs.boundary);
        shift = shift - (shift % BITS);
        ngon -= 2;
        if(shift / BITS <= ngon) {
            ngon -= shift / BITS;
            bs.boundary >>= shift;
            if(bs.boundary == VALENCE - 2) { //We have reached the first node twice
                if(ngon > 0) {               //Check if we have enough nodes to insert from the ngon left
#                   if (VALENCE != 3)
                    bs.boundary = 1ull;
#                   endif
                    bs.boundary <<= BITS * (ngon - 1);
                    bs.size       = ngon;
                    return bs;
                } 
            } else {
                if((bs.boundary & MASK) < VALENCE - 2) {
                    bs.boundary  += 1;
                    bs.boundary <<= BITS * (ngon);
                    bs.size       = size + ngon - shift / BITS;
                    return bs;
                }
            }
        }
    }
    return (struct boundary_size){.boundary = 0, .size = 0};
}

struct boundary_size remove_ngon(inttype boundary, int size, int ngon) {
    uint32_t shift = __builtin_ctzll(boundary);
    shift = shift - (shift % BITS);
    ngon -= shift / BITS;
    if(ngon < 0) return (struct boundary_size){.boundary = 0, .size = 0}; // something like this
    boundary <<= shift;
    if(boundary == 1) {
        
    }

    return (struct boundary_size){.boundary = 0, .size = 0};

    
    /* struct boundary_size bs = {.boundary = 0, .size = 0}; */
    /* unsigned char first = boundary & ~ngon_masks(size); */
    /* // Check if we can add an edge to the first node) */
    /* if(first < (VALENCE - 2) << (BITS * (size - 1))) { */
    /*     bs.boundary = boundary + (1 << (BITS * (size - 1))); */
    /*     uint32_t shift = __builtin_ctzll(ngon_add_compare(size) ^ bs.boundary); */
    /*     shift = shift - (shift % BITS); */
    /*     ngon -= 2; */
    /*     if(shift / BITS <= ngon) { */
    /*         ngon -= shift / BITS; */
    /*         bs.boundary >>= shift; */
    /*         if(bs.boundary == VALENCE - 2) { */
    /*             if(ngon > 0) { */
    /*                 bs.boundary <<= BITS * (ngon - 1); */
    /*                 bs.size       = ngon; */
    /*                 return bs; */
    /*             }  */
    /*         } else { */
    /*             if((bs.boundary & MASK) < VALENCE - 2) { */
    /*                 bs.boundary  += 1; */
    /*                 bs.boundary <<= BITS * (ngon); */
    /*                 bs.size       = size + ngon - shift / BITS; */
    /*                 return bs; */
    /*             } */
    /*         } */
    /*     } */
    /* } */
}

inline char is_mouse(inttype boundary, int size) {
    // A mouse boundary complex has odd length and the head has valence 1
    if((size & 1) == 1 && ((boundary & MASK) == 0)) {
        boundary >>= 1;
        inttype v = boundary;
        inttype s = sizeof(inttype) * 8; // bit size; must be power of 2
        inttype mask = ~0ull;
        while ((s >>= 1ull) >= (1ull << (BITS - 1))) {
            mask ^= (mask << s);
            v = ((v >> s) & mask) | ((v << s) & ~mask);
        }
        v >>= (sizeof(inttype) / BITS) * 8 - size + 1;
        if(boundary + v == (1 << (size - 1)) - 1) {
            return 1;
        }
    }
    return 0;
}

#if 0
const inttype ngon_masks2[] = {
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

const inttype ngon_compare2[] = {
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

struct boundary_size insert_ngon2(inttype boundary, int size, int ngon) {
    struct boundary_size bs = {0, 0};
    unsigned char first = boundary & MASK;
    if(first >= VALENCE - 2) return bs;
    inttype masked_boundary = boundary & ngon_masks2[size];
    /* if(masked_boundary == ngon_compare[size]) { */
        
    /* } else { */
        uint32_t shift = __builtin_ctzll(ngon_compare2[size] ^ (boundary & ngon_masks2[size]));
        shift = shift - (shift % BITS) - BITS;
        ngon -= 2;
        if(shift / BITS > ngon) return bs;
        ngon -= shift;
        bs.boundary   = ((boundary >> shift) & ~MASK);
        bs.boundary  += (1 << BITS);
        bs.boundary <<= BITS * (ngon);
        bs.boundary  += first + 1;
        bs.size       = size + ngon - shift / BITS;
        //bs.boundary  &= (1 << bs.size) - 1;
        //}
    return bs;
}

struct boundary_size remove_ngon2(inttype boundary, int size, int ngon) {
    struct boundary_size bs = {0, 0};
    unsigned char first = boundary & MASK;
    if(first == 0) return bs;
    uint32_t shift = __builtin_ctzll(ngon_add_compare[ngon] ^ (boundary & ngon_masks[ngon]));
    shift = shift - (shift % BITS) - BITS;
    ngon -= 2;
    if(shift / BITS > ngon) return bs;
    ngon -= shift;
    bs.boundary   = ((boundary >> shift) & ~MASK);
    bs.boundary  += (1ull << BITS);
    bs.boundary <<= BITS * (ngon);
    bs.boundary  += first + 1;
    bs.size       = size + ngon - shift / BITS;
    //bs.boundary  &= (1ull << bs.size) - 1;
    return bs;
}

void insert_test() {
    for(int size = 2; size < 6; size++) {
        for(int i = 0; i < (1ull << (size - 1)); i++) {
            for(int j = 3; j < 6; j++) {
                struct boundary_size bs = insert_ngon(i, size, j);
                struct boundary_size bs2 = insert_ngon2(rotl(i, 1, size), size, j);
                bs2.boundary = rotr(bs2.boundary, 1, bs2.size);
                if(bs.size != bs2.size || bs.boundary != bs2.boundary) {
                    printf("Insert %i-gon in %02x[%i]: %04x[%i] %04x[%i]\n", j, i, size, (int)bs.boundary, bs.size, (int)bs2.boundary, bs2.size); 
                    printf("Insert %i-gon in %02x[%i]: %04x[%i] %04x[%i]\n\n", j, i, size,
                           (int)normalize(bs.boundary, bs.size), bs.size, (int)normalize(bs2.boundary, bs2.size), bs2.size);
                }
            }
        }
    }
}
#endif

void remove_test() {
    for(int size = 1; size < 3; size++) {
        for(int i = 0; i < (1ull << size); i++) {
            for(int j = 3; j < 5; j++) {
                struct boundary_size bs = insert_ngon(i, size, j);
                bs = remove_ngon(bs.boundary, bs.size, j);
                printf("%x[%02i]\n", i, size);
                printf("%lx[%02i]\n\n", bs.boundary, bs.size);
            }
        }
    }
}

void mouse_test() {
    inttype mouse;
    int size;
    mouse = 0x1A8; size = 9;
    printf("Mouse: %lx[%i] %c\n", mouse, size, is_mouse(mouse, size) ? 'y' : 'n');
}

int small_ngon = 5;
int large_ngon = 7;
#define MAX_SIZE 20

queue_t queues[MAX_SIZE];
uint8_t* database[MAX_SIZE];

void database_init() {
    for(int i = 1; i < MAX_SIZE; i++) {
        uint64_t size = ((1ull << i) + 15) / 8;
        database[i] = malloc(size);
        if(database[i] <= 0) {
            printf("Allocation of Database[%i] failed\n", i);
        } else {
            memset(database[i], 0, size);
        }
    }
}

void database_deinit() {
    for(int i = 0; i < MAX_SIZE; i++) {
        if(database[i] > 0) {
            free(database[i]);
        }
    }
}

uint8_t database_contains(inttype boundary, int size) {
    return (database[size][boundary >> 3] & (1ull << (boundary & 7))) >> (boundary & 7);
}

void database_add(inttype boundary, int size) {
    //printf("[%02i] %lx\n", size, boundary);
    database[size][boundary >> 3] |= 1ull << (boundary & 7);
}


void* search_worker(void* dummy) {

    return NULL;
}

void clever_search2(inttype boundary, int size) {
    database_init();
    for(int i = 1; i < MAX_SIZE; i++) {
        queue_init(&queues[i]);
    }
    queue_enqueue(&queues[size], normalize(boundary, size));
    //pthread_t threads[NUM_THREADS];
    for(int i = 0; i < NUM_THREADS; i++) {
        //pthread_create(&threads[i], NULL, search_worker, NULL);
    }
}


void build_database(inttype boundary, int size) {
    database_init();
    for(int i = 0; i < MAX_SIZE; i++) {
        queue_init(&queues[i]);
    }
    queue_enqueue(&queues[size], normalize(boundary, size));
    struct boundary_size bs;
    int found = 0;
start:
    for(int size = 1; size < MAX_SIZE; size++) {
        if(!queue_is_empty(&queues[size])) {
            boundary = queue_dequeue(&queues[size]);
            if(!database_contains(boundary, size)) {
                database_add(boundary, size);
                check_boundary_size(boundary, size);
                for(int j = 0; j < size; j++) {
                    if(is_mouse(boundary, size)) {
                        printf("MOUSE [%02i] %lx\n", size, boundary);
                    }
                    bs = insert_ngon(boundary, size, small_ngon);
                    if(bs.size != 0 && bs.size < MAX_SIZE) {
                        inttype normalized = normalize(bs.boundary, bs.size);
                        if(bs.boundary == 0x41 && bs.size == 6)
                            printf("Found [%02i] %lx during inserting %i in [%02i] %lx\n", bs.size, normalized, small_ngon, size, boundary);
                        queue_enqueue(&queues[bs.size], normalized);
                    }
                    bs = insert_ngon(boundary, size, large_ngon);
                    if(bs.size != 0 && bs.size < MAX_SIZE) {
                        inttype normalized = normalize(bs.boundary, bs.size);
                        if(bs.boundary == 0x41 && bs.size == 6)
                            printf("Found [%02i] %lx during inserting %i in [%02i] %lx\n", bs.size, normalized, small_ngon, size, boundary);
                        queue_enqueue(&queues[bs.size], normalized);
                    }
                    boundary = rotl(boundary, BITS, size);
                }
            }
            goto start;
        }
    }
    for(int i = 0; i < MAX_SIZE; i++) {
        queue_deinit(&queues[i]);
    }
}


int main(int argc, char* argv[]) {
    inttype a = 0;
    if(argc == 3) {
        small_ngon = atoi(argv[1]);
        large_ngon = atoi(argv[2]);
    } else {
        small_ngon = 5;
        large_ngon = 7;
    }

    /* struct boundary_size bs = insert_ngon(0xc50, 12, 7); */
    /* printf("%lx[%i]\n",bs.boundary, bs.size); */
    /* printf("%lx[%i]\n",normalize(bs.boundary, bs.size), bs.size); */
    
    
    //insert_test();
    //queue_test2();
    //debug_search(a, 6);
    //remove_test();
    
    build_database(a, 1);

    database_deinit();
}
