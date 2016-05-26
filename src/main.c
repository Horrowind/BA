#include <assert.h>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <errno.h>

#include <pthread.h>

//#include <immintrin.h>


#define VALENCE        3
#define BITS           (VALENCE < 4 ? 1 : 2)
#define LAST_NODE_MASK ((1ull << BITS) - 1)
#define inttype        uint64_t

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

#if 0
inline
#endif
void check_boundary_size(inttype boundary, int size) {
#   ifndef NDEBUG
    if(boundary != 0 && sizeof(inttype) * 8 - __builtin_clzll(boundary) > size * BITS) {
        printf("Boundary %lx is larger then size %i\n", boundary, size);
        exit(0);
    }
#   endif
}

#if 0
inline
#endif
inttype ngon_masks(int size) {
    assert(size <= (BITS - 1 ? 14 : 31));
    return (LAST_NODE_MASK << (BITS *  (size - 1)));
}

#if 0
inline
#endif
inttype ngon_add_compare(int size) {
    assert(size <= (BITS - 1 ? 14 : 31));
    return (VALENCE - 2) * (((1ull << (BITS *  (size - 1))) - 1) / LAST_NODE_MASK);
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

struct boundary_size insert_ngon(inttype boundary, int size, int ngon) {         // Example: boundary = 0b01001, size = 5, ngon = 3
                                                                                 //                                   and valence 3
    struct boundary_size bs = {.boundary = 0, .size = 0};                        // Initialize...
    if(boundary >> (BITS * (size - 1)) < VALENCE - 2) {                          // Get the first node (0b*0*1001) and check if one
                                                                                 //                               could add an ngon
        bs.boundary = boundary + (1 << (BITS * (size - 1)));                     // bs.boundary = 0b*1*1001
        uint32_t shift = __builtin_ctzll(ngon_add_compare(size) ^ bs.boundary);  // shift = 1 (the number of trailing 1s of 0b11001
                                                                                 //                                           is 1) 
        shift = shift - (shift % BITS);                                          // in the case of valence > 3, we round down (here
                                                                                 //                                     shift is 1)
        ngon -= 2;                                                               // ngon = 1
        if(shift / BITS <= ngon) {                                               // Test if one can insert all the necessary nodes
            ngon -= shift / BITS;                                                // ngon = 0
            bs.boundary >>= shift;                                               // bs.boundary = 0b1100
            if(bs.boundary == VALENCE - 2) {                                     // Check if we have reached the first node for the
                                                                                 //                   second time (here: we do not)
                if(ngon > 0) {                                                   // Check if one has  enough nodes to insert to the
                                                                                 //                                left of the ngon
                    // Case #1
#                   if (VALENCE != 3)
                    bs.boundary = 1ull;
#                   endif
                    bs.boundary <<= BITS * (ngon - 1);
                    bs.size       = ngon;
                } 
            } else {
                if((bs.boundary & LAST_NODE_MASK) < VALENCE - 2) {               // Check if one could add something to the last node (0b110*0*, here: yes)
                    bs.boundary  += 1;                                           // bs.boundary = 0b1101
                    bs.boundary <<= BITS * (ngon);                               // bs.boundary = 0b1101
                    bs.size       = size + ngon - shift / BITS;                  // bs.size = 5 + 0 - 1 = 4
                }
            }
        } else {
            bs.boundary = 0;
        }
    }
    return bs;
}

struct boundary_size remove_ngon(inttype boundary, int size, int ngon) {        // Example: boundary = 0b1101, size = 4, ngon = 3
    if((boundary & (boundary - 1)) == 0 && ngon >= size + 2) {                    // Check if we are in Case #1 above
        
    } else {


        uint32_t shift = __builtin_ctzll(boundary);                                 // shift = 1 (the number of trailing 1s of 0b1101 is 1)
        shift = shift - (shift % BITS);                                             // round down shift if valence > 3 (here shift = 1)
        ngon -= shift / BITS;                                                       // ngon = 2
        if(ngon < 0) return (struct boundary_size){.boundary = 0, .size = 0};       // something like this
        boundary <<= shift;
        if((boundary & (boundary - 1)) == 0 && ngon ) {                                             // if boundary is not a power of 2
        
        }
    
        return (struct boundary_size){.boundary = 0, .size = 0};
    
    }
    return (struct boundary_size){.boundary = 0, .size = 0};
}


#if 0
inline
#endif
char is_mouse(inttype boundary, int size) {
    // A mouse boundary complex has odd length and the head has valence 1
    if((size & 1) == 1 && ((boundary & LAST_NODE_MASK) == 0)) {
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
    LAST_NODE_MASK * ((1 << (BITS *  1)) - 1) / LAST_NODE_MASK << BITS,
    LAST_NODE_MASK * ((1 << (BITS *  2)) - 1) / LAST_NODE_MASK << BITS,
    LAST_NODE_MASK * ((1 << (BITS *  3)) - 1) / LAST_NODE_MASK << BITS,
    LAST_NODE_MASK * ((1 << (BITS *  4)) - 1) / LAST_NODE_MASK << BITS,
    LAST_NODE_MASK * ((1 << (BITS *  5)) - 1) / LAST_NODE_MASK << BITS,
    LAST_NODE_MASK * ((1 << (BITS *  6)) - 1) / LAST_NODE_MASK << BITS,
    LAST_NODE_MASK * ((1 << (BITS *  7)) - 1) / LAST_NODE_MASK << BITS,
    LAST_NODE_MASK * ((1 << (BITS *  8)) - 1) / LAST_NODE_MASK << BITS,
    LAST_NODE_MASK * ((1 << (BITS *  9)) - 1) / LAST_NODE_MASK << BITS,
    LAST_NODE_MASK * ((1 << (BITS * 10)) - 1) / LAST_NODE_MASK << BITS,
    LAST_NODE_MASK * ((1 << (BITS * 11)) - 1) / LAST_NODE_MASK << BITS,
    LAST_NODE_MASK * ((1 << (BITS * 12)) - 1) / LAST_NODE_MASK << BITS,
    LAST_NODE_MASK * ((1 << (BITS * 13)) - 1) / LAST_NODE_MASK << BITS,
    LAST_NODE_MASK * ((1 << (BITS * 14)) - 1) / LAST_NODE_MASK << BITS,
#if (BITS == 1)
    LAST_NODE_MASK * ((1 << (BITS * 15)) - 1) / LAST_NODE_MASK << BITS,
    LAST_NODE_MASK * ((1 << (BITS * 16)) - 1) / LAST_NODE_MASK << BITS,
    LAST_NODE_MASK * ((1 << (BITS * 17)) - 1) / LAST_NODE_MASK << BITS,
    LAST_NODE_MASK * ((1 << (BITS * 18)) - 1) / LAST_NODE_MASK << BITS,
    LAST_NODE_MASK * ((1 << (BITS * 19)) - 1) / LAST_NODE_MASK << BITS,
    LAST_NODE_MASK * ((1 << (BITS * 20)) - 1) / LAST_NODE_MASK << BITS,
    LAST_NODE_MASK * ((1 << (BITS * 21)) - 1) / LAST_NODE_MASK << BITS,
    LAST_NODE_MASK * ((1 << (BITS * 22)) - 1) / LAST_NODE_MASK << BITS,
    LAST_NODE_MASK * ((1 << (BITS * 23)) - 1) / LAST_NODE_MASK << BITS,
    LAST_NODE_MASK * ((1 << (BITS * 24)) - 1) / LAST_NODE_MASK << BITS,
    LAST_NODE_MASK * ((1 << (BITS * 25)) - 1) / LAST_NODE_MASK << BITS,
    LAST_NODE_MASK * ((1 << (BITS * 26)) - 1) / LAST_NODE_MASK << BITS,
    LAST_NODE_MASK * ((1 << (BITS * 27)) - 1) / LAST_NODE_MASK << BITS,
    LAST_NODE_MASK * ((1 << (BITS * 28)) - 1) / LAST_NODE_MASK << BITS,
    LAST_NODE_MASK * ((1 << (BITS * 29)) - 1) / LAST_NODE_MASK << BITS,
    LAST_NODE_MASK * ((1 << (BITS * 30)) - 1) / LAST_NODE_MASK << BITS,
#endif
};

const inttype ngon_compare2[] = {
    (VALENCE - 2) * ((1 << (BITS *  1)) - 1) / LAST_NODE_MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS *  2)) - 1) / LAST_NODE_MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS *  3)) - 1) / LAST_NODE_MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS *  4)) - 1) / LAST_NODE_MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS *  5)) - 1) / LAST_NODE_MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS *  6)) - 1) / LAST_NODE_MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS *  7)) - 1) / LAST_NODE_MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS *  8)) - 1) / LAST_NODE_MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS *  9)) - 1) / LAST_NODE_MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS * 10)) - 1) / LAST_NODE_MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS * 11)) - 1) / LAST_NODE_MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS * 12)) - 1) / LAST_NODE_MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS * 13)) - 1) / LAST_NODE_MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS * 14)) - 1) / LAST_NODE_MASK << BITS,
#if (BITS == 1)
    (VALENCE - 2) * ((1 << (BITS * 15)) - 1) / LAST_NODE_MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS * 16)) - 1) / LAST_NODE_MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS * 17)) - 1) / LAST_NODE_MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS * 18)) - 1) / LAST_NODE_MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS * 19)) - 1) / LAST_NODE_MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS * 20)) - 1) / LAST_NODE_MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS * 21)) - 1) / LAST_NODE_MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS * 22)) - 1) / LAST_NODE_MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS * 23)) - 1) / LAST_NODE_MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS * 24)) - 1) / LAST_NODE_MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS * 25)) - 1) / LAST_NODE_MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS * 26)) - 1) / LAST_NODE_MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS * 27)) - 1) / LAST_NODE_MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS * 28)) - 1) / LAST_NODE_MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS * 29)) - 1) / LAST_NODE_MASK << BITS,
    (VALENCE - 2) * ((1 << (BITS * 30)) - 1) / LAST_NODE_MASK << BITS,
#endif
};

struct boundary_size insert_ngon2(inttype boundary, int size, int ngon) {
    struct boundary_size bs = {0, 0};
    unsigned char first = boundary & LAST_NODE_MASK;
    if(first >= VALENCE - 2) return bs;
    inttype masked_boundary = boundary & ngon_masks2[size];
    /* if(masked_boundary == ngon_compare[size]) { */
        
    /* } else { */
        uint32_t shift = __builtin_ctzll(ngon_compare2[size] ^ (boundary & ngon_masks2[size]));
        shift = shift - (shift % BITS) - BITS;
        ngon -= 2;
        if(shift / BITS > ngon) return bs;
        ngon -= shift;
        bs.boundary   = ((boundary >> shift) & ~LAST_NODE_MASK);
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
    unsigned char first = boundary & LAST_NODE_MASK;
    if(first == 0) return bs;
    uint32_t shift = __builtin_ctzll(ngon_add_compare[ngon] ^ (boundary & ngon_masks[ngon]));
    shift = shift - (shift % BITS) - BITS;
    ngon -= 2;
    if(shift / BITS > ngon) return bs;
    ngon -= shift;
    bs.boundary   = ((boundary >> shift) & ~LAST_NODE_MASK);
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
#define MAX_SIZE 30

uint8_t* database[MAX_SIZE];

void database_init() {
    for(int i = 1; i < MAX_SIZE; i++) {
        uint64_t size = i < 3 ? 1 : 1ull << (i - 3);
        if(size < sysconf(_SC_PAGESIZE)) {
            database[i] = malloc(size);
            if(database[i] <= 0) {
                printf("Allocation of Database[%i] failed (size: 0x%lx)\n", i, size);
            } else {
                memset(database[i], 0, size);
            }
        } else {
            database[i] = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            if(database[i] == MAP_FAILED) {
                char* error_string = strerror(errno);
                printf("Allocation of Database[%i] failed (size: 0x%lx): %s\n", i, size, error_string);
            }
        }
    }
}

void database_deinit() {
    for(int i = 0; i < MAX_SIZE; i++) {
        if(database[i] > 0) {
            uint64_t size = i < 3 ? 1 : 1ull << (i - 3);
            if(size < sysconf(_SC_PAGESIZE))  {
                free(database[i]);
            } else {
                munmap(database[i], size);
            }
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


/* void clever_search2(inttype boundary, int size) { */
/*     database_init(); */
/*     for(int i = 1; i < MAX_SIZE; i++) { */
/*         queue_init(&queues[i]); */
/*     } */
/*     queue_enqueue(&queues[size], normalize(boundary, size)); */
/*     //pthread_t threads[NUM_THREADS]; */
/*     for(int i = 0; i < NUM_THREADS; i++) { */
/*         //pthread_create(&threads[i], NULL, search_worker, NULL); */
/*     } */
/* } */

pthread_mutex_t number_of_active_threads_mutex;
int number_of_active_threads;

struct thread_data {
    queue_t queues[MAX_SIZE];
};
    
void thread_data_init(struct thread_data* data) {
    for(int i = 0; i < MAX_SIZE; i++) {
        queue_init(&data->queues[i]);
    }
}

#define NUM_THREADS 2

void* working_thread(void* thread_data_ptr) {
    pthread_detach(pthread_self());
    pthread_mutex_lock(&number_of_active_threads_mutex);
    number_of_active_threads++;
    pthread_mutex_unlock(&number_of_active_threads_mutex);
    struct thread_data data = *(struct thread_data*)thread_data_ptr;
    free(thread_data_ptr);
    
    
    
    struct boundary_size bs;
    int found = 0;
start:
    if(number_of_active_threads < NUM_THREADS) {
        int res = pthread_mutex_trylock(&number_of_active_threads_mutex);
        if(res == 0) {
            // We have the mutex
            if(number_of_active_threads < NUM_THREADS) {
                struct thread_data* new_data = malloc(sizeof(struct thread_data));
                for(int i = 0; i < MAX_SIZE; i++) {
                    queue_init(&new_data->queues[i]);
                    int tmp = data.queues[i].head - data.queues[i].tail;
                    int workload = (tmp % data.queues[i].length + data.queues[i].length) % data.queues[i].length;
                    int half_of_the_work = (workload + 1) / 2;
                    new_data->queues[i].data = malloc(sizeof(inttype) * half_of_the_work);
                    for(int j = 0; j < half_of_the_work; j++) {
                        queue_enqueue(&new_data->queues[i], queue_dequeue(&data.queues[i]));
                    }
                }
                pthread_t dummy;
                pthread_create(&dummy, NULL, working_thread, new_data);
            }
            pthread_mutex_unlock(&number_of_active_threads_mutex);
        } else if(res != EBUSY) {
            // An error occured:
            assert(0 && "Error when trying to lock the mutex");
        }
    }
    
    for(int size = 1; size < MAX_SIZE; size++) {
        if(!queue_is_empty(&data.queues[size])) {
            inttype boundary = queue_dequeue(&data.queues[size]);
            if(!database_contains(boundary, size)) {
                check_boundary_size(boundary, size);
                for(int j = 0; j < size; j++) {
//#ifndef NDEBUG
                    if(is_mouse(boundary, size)) {
                        printf("MOUSE [%02i] %lx\n", size, boundary);
                    }
//#endif
                    bs = insert_ngon(boundary, size, small_ngon);
                    if(bs.size != 0 && bs.size < MAX_SIZE) {
                        inttype normalized = normalize(bs.boundary, bs.size);
                        queue_enqueue(&data.queues[bs.size], normalized);
                    }
                    bs = insert_ngon(boundary, size, large_ngon);
                    if(bs.size != 0 && bs.size < MAX_SIZE) {
                        inttype normalized = normalize(bs.boundary, bs.size);
                        queue_enqueue(&data.queues[bs.size], normalized);
                    }
                    boundary = rotl(boundary, BITS, size);
                }
                database_add(boundary, size);
            }
            goto start;
        }
    }
    for(int i = 0; i < MAX_SIZE; i++) {
        queue_deinit(&data.queues[i]);
    }

    pthread_mutex_lock(&number_of_active_threads_mutex);
    number_of_active_threads--;
    pthread_mutex_unlock(&number_of_active_threads_mutex);
    return NULL;
}



long task_ids[NUM_THREADS];
int main(int argc, char* argv[]) {

    pthread_mutex_init(&number_of_active_threads_mutex, NULL);
    number_of_active_threads = 0;
    
    inttype a = 0;
    if(argc == 3) {
        small_ngon = atoi(argv[1]);
        large_ngon = atoi(argv[2]);
    } else {
        small_ngon = 5;
        large_ngon = 7;
    }

    database_init();
    
    struct thread_data* new_data = malloc(sizeof(struct thread_data));
    for(int i = 0; i < MAX_SIZE; i++) {
        queue_init(&new_data->queues[i]);
    }
    queue_enqueue(&new_data->queues[1], 0);
    pthread_t dummy;
    pthread_create(&dummy, NULL, working_thread, new_data);

    pthread_exit(0);
}
