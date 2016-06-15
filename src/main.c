#include <assert.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <errno.h>

#include <pthread.h>

#include "util.h"
#include "test.c"

#ifndef __OPTIMIZE__
#define inline
#endif

#define NUM_THREADS 2
int small_ngon = 5;
int large_ngon = 7;
#define MAX_SIZE 30

#define VALENCE        3
#define BITS           (VALENCE < 4 ? 1 : 2)
#define LAST_NODE_MASK ((1ull << BITS) - 1)

#define rotl(value, shift, size) (((value << (shift * BITS)) & ((1ull << (size * BITS)) - 1)) | (value >> ((size - shift)  * BITS)))
#define rotr(value, shift, size) (((value >> (shift * BITS)) | (value << ((size - shift) * BITS))) & ((1ull << (size * BITS)) - 1))

void
inline check_boundary_size(boundary_t boundary, int size) {
#   ifndef NDEBUG
    if(boundary != 0 && sizeof(boundary) * 8 - __builtin_clzll(boundary) > size * BITS) {
        printf("Boundary %lx is larger then size %i\n", boundary, size);
        exit(0);
    }
#   endif
}

inline
boundary_t normalize(boundary_t boundary, int size) {
   boundary_t max = boundary;
    for(int i = 0; i < size; i++) {
        boundary = rotl(boundary, 1, size);
        if(max < boundary) max = boundary;
    }
    return max;
}


void write_bits(boundary_t in) {
    for(int i = 63; i >= 0; i--) {
        printf("%i", (int)((in >> i) & 1));
    }
    printf("\n");
}

inline
boundary_size_t insert_ngon(boundary_t boundary, int size, int ngon) {
    boundary_size_t bs = {.boundary = 0, .size = 0};
    if((boundary >> (BITS * (size - 1))) < VALENCE - 2) {
        u32 s;
        if(VALENCE != 4) {
            s = __builtin_ctzll(~boundary) / BITS;
        } else {
            s = __builtin_ctzll(~(boundary | 0x5555555555555555)) / BITS;
        }
        if(s != size - 1) {
            if(s <= ngon - 2) {
                bs.boundary   = boundary + (1ull << ((size - 1) * BITS));
                bs.boundary >>= s * BITS;
                bs.boundary  += 1ull;
                bs.boundary <<= (ngon - 2 - s) * BITS;
                bs.size       = size + ngon - 2 - 2 * s;
            }
        } else {
            if((boundary >> (BITS * (size - 1))) < VALENCE - 3) {
                if(size + 1 <= ngon) {
                    bs.boundary   = (boundary >> (BITS * (size - 1))) + 2;
                    bs.boundary <<= (ngon - size - 1) * BITS;
                    bs.size       = ngon - size;
                }
            } else {
                if(size + 3 <= ngon) {
                    bs.boundary   = 1 << (ngon - size - 3) * BITS;
                    bs.size       = ngon - size - 2;
                }
            }
        }        
    }
    return bs;   
}



inline
boundary_size_t remove_ngon(boundary_t boundary, int size, int ngon) {
    boundary_size_t bs = {.boundary = 0, .size = 0};
    if((boundary >> (BITS * (size - 1))) != 0) {
        u32 s = __builtin_ctzll(boundary) / BITS;
        if(s != size - 1) {
            if(s <= ngon - 2) {
                bs.boundary   = boundary - (1ull << ((size - 1) * BITS));
                bs.boundary >>= s * BITS;
                bs.boundary  -= 1;
                bs.boundary <<= (ngon - 2 - s) * BITS;
                bs.boundary  |= (((1 << (ngon - 2 - s) * BITS) - 1) / LAST_NODE_MASK) * (VALENCE - 2);
                bs.size       = size + ngon - 2 - 2 * s;
            }
        } else { // Overlapping case
            if((boundary >> (BITS * (size - 1))) < VALENCE - 3) { // Overlapping with a single node
                if(size + 1 <= ngon) {
                    bs.boundary   = (boundary >> (BITS * (size - 1))) - 2;
                    bs.boundary <<= (ngon - size - 1) * BITS;
                    bs.boundary  |= ((1 << ((ngon - size - 1) * BITS)) - 1) / LAST_NODE_MASK * (VALENCE - 2);
                    bs.size       = ngon - size;
                }
            } else { // Overlapping with a single edge
                if(size + 3 <= ngon) { 
                    bs.boundary   = (VALENCE - 3) << ((ngon - size - 3) * BITS);
                    write_bits(bs.boundary);
                    bs.boundary  |= (((1 << ((ngon - size - 3) * BITS)) - 1) / LAST_NODE_MASK) * (VALENCE - 2);
                    write_bits(bs.boundary);
                    bs.size       = ngon - size - 2;
                }
            }
        }        
    }
    return bs;   
}

inline
char is_mouse(boundary_t boundary, int size) {
    // A mouse boundary complex has odd length and the head has valence 1
    if((size & 1) == 1 && ((boundary & LAST_NODE_MASK) == 0)) {
        boundary >>= 1;
        boundary_t v = boundary;
        boundary_t s = sizeof(boundary) * 8; // bit size; must be power of 2
        boundary_t mask = ~0ull;
        while ((s >>= 1ull) >= (1ull << (BITS - 1))) {
            mask ^= (mask << s);
            v = ((v >> s) & mask) | ((v << s) & ~mask);
        }
        v >>= (sizeof(boundary) / BITS) * 8 - size + 1;
        if(boundary + v == (1 << (size - 1)) - 1) {
            return 1;
        }
    }
    return 0;
}



u8* database[MAX_SIZE];

void database_init() {
    for(int i = 1; i < MAX_SIZE; i++) {
        u64 size = (i < 3 ? 1 : 1ull << (i - 3)) * BITS;
        if(size < sysconf(_SC_PAGESIZE)) {
            database[i] = malloc(size);
            assert(database[i] != 0 && "Allocation of Database failed\n");
            memset(database[i], 0, size);
        } else {
            database[i] = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            assert(database[i] != MAP_FAILED && "Allocation of Database failed\n");
        }
    }
}

void database_deinit() {
    for(int i = 0; i < MAX_SIZE; i++) {
        if(database[i] > 0) {
            u64 size = (i < 3 ? 1 : 1ull << (i - 3)) * BITS;
            if(size < sysconf(_SC_PAGESIZE))  {
                free(database[i]);
            } else {
                munmap(database[i], size);
            }
        }
    }
}

b32 database_contains(boundary_t boundary, int size) {
    return (database[size][boundary >> 3] & (1ull << (boundary & 7))) >> (boundary & 7);
}

void database_add(boundary_t boundary, int size) {
    database[size][boundary >> 3] |= 1ull << (boundary & 7);
}

enum thread_state {
    THREAD_STATE_TERMINATING,
    THREAD_STATE_WAITING,
    THREAD_STATE_RUNNING,
    THREAD_STATE_CAN_SHARE,
};

struct thread_data {
    pthread_t thread_id;
    pthread_cond_t thread_cond;
    page_allocator_t page_allocator;
    queue_t queues[MAX_SIZE];
    enum thread_state state;
};

struct {
    int number_of_active_threads;
    struct thread_data data[NUM_THREADS];
    pthread_mutex_t mutex;
} thread_manager;

    
void thread_data_init(struct thread_data* data) {
    page_allocator_init(&data->page_allocator);
    for(int i = 0; i < MAX_SIZE; i++) {
        queue_init(&data->queues[i], &data->page_allocator);
    }
    data->state = THREAD_STATE_RUNNING;
}


void* working_thread(void* thread_number_disguised_as_ptr) {

    long thread_number = (long)thread_number_disguised_as_ptr;
    struct thread_data* data = &thread_manager.data[thread_number];

    while(1) {
        if(thread_manager.number_of_active_threads < NUM_THREADS && data->state == THREAD_STATE_CAN_SHARE) {
            int res = pthread_mutex_trylock(&thread_manager.mutex);
            if(res == 0) {
                if(thread_manager.number_of_active_threads < NUM_THREADS) {
                    ulong next_waiting_thread = (thread_number + 1) % NUM_THREADS;
                    while(thread_manager.data[next_waiting_thread].state != THREAD_STATE_WAITING) {
                        next_waiting_thread = (next_waiting_thread + 1) % NUM_THREADS;
                    }

                    struct thread_data* other_data = &thread_manager.data[next_waiting_thread];
                    
                    for(int i = 1; i < MAX_SIZE; i++) {
                        if(data->queues[i].head_page != data->queues[i].tail_page) {
                            queue_move_first_page(&data->queues[i], &other_data->queues[i]);
                        }
                    }
                    other_data->state = THREAD_STATE_RUNNING;
                    thread_manager.number_of_active_threads++;

                    pthread_cond_signal(&other_data->thread_cond);
                }
                pthread_mutex_unlock(&thread_manager.mutex);
            } else {
                assert(res == EBUSY && "Error when trying to lock the mutex");
            }
        }
        
        int has_no_items = 1;
        boundary_size_t bs;
        for(int size = 1; size < MAX_SIZE; size++) {
            if(!queue_is_empty(&data->queues[size])) {
                boundary_t boundary = queue_dequeue(&data->queues[size]);
                check_boundary_size(boundary, size);
                if(!database_contains(boundary, size)) {
#ifndef NDEBUG
                    printf("Thread %li found [%02i] %lx\n", thread_number, size, boundary);
#endif
                    for(int j = 0; j < size; j++) {
                        if(is_mouse(boundary, size)) {
                            printf("Thread %li found MOUSE [%02i] %lx\n", thread_number, size, boundary);
                        }
                        bs = insert_ngon(boundary, size, small_ngon);
                        if(bs.size != 0 && bs.size < MAX_SIZE) {
                            boundary_t normalized = normalize(bs.boundary, bs.size);
                            queue_enqueue(&data->queues[bs.size], normalized);
                        }
                        bs = insert_ngon(boundary, size, large_ngon);
                        if(bs.size != 0 && bs.size < MAX_SIZE) {
                            boundary_t normalized = normalize(bs.boundary, bs.size);
                            queue_enqueue(&data->queues[bs.size], normalized);
                        }
                        boundary = rotl(boundary, BITS, size);
                    }
                    database_add(boundary, size);
                }
                has_no_items = 0;
                if(data->queues[size].head_page != data->queues[size].tail_page) data->state = THREAD_STATE_CAN_SHARE;
                break;
            }
            
        }
        
        if(has_no_items) {
            pthread_mutex_lock(&thread_manager.mutex);
            thread_manager.number_of_active_threads--;
            if(thread_manager.number_of_active_threads == 0) {
                for(long other_thread = (thread_number + 1) % NUM_THREADS;
                    other_thread != thread_number;
                    other_thread = (other_thread + 1) % NUM_THREADS) {
                    thread_manager.data[other_thread].state = THREAD_STATE_TERMINATING;
                    pthread_cond_signal(&thread_manager.data[other_thread].thread_cond);
                }
                pthread_mutex_unlock(&thread_manager.mutex);
                page_allocator_deinit(&data->page_allocator);
                pthread_exit(NULL);
            }
            data->state = THREAD_STATE_WAITING;

            while(data->state == THREAD_STATE_WAITING) {
                pthread_cond_wait(&data->thread_cond, &thread_manager.mutex);
            }
            if(data->state == THREAD_STATE_TERMINATING) {
                pthread_mutex_unlock(&thread_manager.mutex);
                page_allocator_deinit(&data->page_allocator);
                pthread_exit(NULL);
            }
            pthread_mutex_unlock(&thread_manager.mutex);
        }
    };
}



long task_ids[NUM_THREADS];
int main(int argc, char* argv[]) {
    if(argc == 3) {
        small_ngon = atoi(argv[1]);
        large_ngon = atoi(argv[2]);
    } else {
        small_ngon = 5;
        large_ngon = 7;
    }

    database_init();

    pthread_mutex_init(&thread_manager.mutex, NULL);
    thread_manager.number_of_active_threads = NUM_THREADS;

    thread_data_init(&thread_manager.data[0]);
    
    queue_enqueue(&thread_manager.data[0].queues[1], 0);
    thread_manager.data[0].state = THREAD_STATE_RUNNING;

    pthread_cond_init(&thread_manager.data[0].thread_cond, NULL);
    
    for(long thread_number = 1; thread_number < NUM_THREADS; thread_number++) {
        thread_data_init(&thread_manager.data[thread_number]);
        pthread_cond_init(&thread_manager.data[thread_number].thread_cond, NULL);
        pthread_create(&thread_manager.data[thread_number].thread_id, NULL, working_thread, (void*)thread_number);
    }

    pthread_create(&thread_manager.data[0].thread_id, NULL, working_thread, (void*)0);
    pthread_mutex_unlock(&thread_manager.mutex);
    for(int thread_number = 0; thread_number < NUM_THREADS; thread_number++) {
        printf("Waiting for thread %i...\n", thread_number);
        pthread_join(thread_manager.data[thread_number].thread_id, NULL);
        printf("Waiting for thread %i successfully!\n", thread_number);
    }

    /* for(int i = 1; i < MAX_SIZE; i++) { */
    /*     int s = 0; */
    /*     for(int j = 0; j < (1 << i); j++) { */
    /*         if(database_contains(j, i)) s++; */
    /*     } */
    /* } */
}
