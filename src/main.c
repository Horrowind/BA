#include <assert.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>

#include "database.h"
#include "pool.h"
#include "stack.h"

typedef enum thread_state {
    THREAD_STATE_TERMINATING,
    THREAD_STATE_WAITING,
    THREAD_STATE_RUNNING,
    THREAD_STATE_CAN_SHARE,
} thread_state_t;

typedef struct {
    pthread_t thread_id;
    pthread_cond_t thread_cond;
    page_allocator_t page_allocator;
    queue_t queues[MAX_SIZE];
    enum thread_state state;
} thread_data_t;

typedef struct {
    int number_of_active_threads;
    thread_data_t thread_data[NUM_THREADS];
    pthread_mutex_t mutex;
    database_t database;
} thread_manager_t;

    
void thread_manager_init(thread_manager_t* thread_manager, database_t database) {
    thread_manager->number_of_active_threads = NUM_THREADS;
    thread_manager->database = database;
    pthread_mutex_init(&thread_manager->mutex, NULL);
    for(int thread = 0; thread < NUM_THREADS; thread++) { 
        page_allocator_init(&thread_manager->thread_data[thread].page_allocator);
        for(int size = 0; size < MAX_SIZE; size++) {
            queue_init(&thread_manager->thread_data[thread].queues[size], &thread_manager->thread_data[thread].page_allocator);
        }
        thread_manager->thread_data[thread].state = THREAD_STATE_RUNNING;
        pthread_cond_init(&thread_manager->thread_data[thread].thread_cond, NULL);
    }
}

void thread_try_sharing(thread_manager_t* thread_manager, long thread_number) {
    thread_data_t* data = &thread_manager->thread_data[thread_number];
    int res = pthread_mutex_trylock(&thread_manager->mutex);
    if(res == 0) {
        if(thread_manager->number_of_active_threads < NUM_THREADS) {
            ulong next_waiting_thread = (thread_number + 1) % NUM_THREADS;
            while(thread_manager->thread_data[next_waiting_thread].state != THREAD_STATE_WAITING) {
                next_waiting_thread = (next_waiting_thread + 1) % NUM_THREADS;
            }

            thread_data_t* other_data = &thread_manager->thread_data[next_waiting_thread];
                    
            for(int i = 1; i < MAX_SIZE; i++) {
                if(data->queues[i].head_page != data->queues[i].tail_page) {
                    queue_move_first_page(&data->queues[i], &other_data->queues[i]);
                }
            }
            other_data->state = THREAD_STATE_RUNNING;
            thread_manager->number_of_active_threads++;

            pthread_cond_signal(&other_data->thread_cond);
        }
        pthread_mutex_unlock(&thread_manager->mutex);
    } else {
        assert(res == EBUSY && "Error when trying to lock the mutex");
    }
}
    
void thread_wait(thread_manager_t* thread_manager, ulong thread_number) {
    thread_data_t* data = &thread_manager->thread_data[thread_number];
    pthread_mutex_lock(&thread_manager->mutex);
    thread_manager->number_of_active_threads--;
    if(thread_manager->number_of_active_threads == 0) {
        for(long other_thread = (thread_number + 1) % NUM_THREADS;
            other_thread != thread_number;
            other_thread = (other_thread + 1) % NUM_THREADS) {
            thread_manager->thread_data[other_thread].state = THREAD_STATE_TERMINATING;
            pthread_cond_signal(&thread_manager->thread_data[other_thread].thread_cond);
        }
        pthread_mutex_unlock(&thread_manager->mutex);
        page_allocator_deinit(&data->page_allocator);
        pthread_exit(NULL);
    }
    data->state = THREAD_STATE_WAITING;

    while(data->state == THREAD_STATE_WAITING) {
        pthread_cond_wait(&data->thread_cond, &thread_manager->mutex);
    }
    if(data->state == THREAD_STATE_TERMINATING) {
        pthread_mutex_unlock(&thread_manager->mutex);
        page_allocator_deinit(&data->page_allocator);
        pthread_exit(NULL);
    }
    pthread_mutex_unlock(&thread_manager->mutex);
}

typedef struct {
    int        ngon;
    int        rotation;
} search_stack_entry_t;

generate_stack(search_stack, search_stack_entry_t);
search_stack_t search_stack;

b32 search_backward_in_database(database_t database, boundary_t start, boundary_t goal) {
    boundary_t boundary = start;
    search_stack_do_empty(&search_stack);
    search_stack_push(&search_stack, (search_stack_entry_t){.ngon = small_ngon, .rotation = 0});
    int already_searched = -1;
    while(1) {
//#ifndef NDEBUG
        for(int i = 0; i < search_stack.fill; i++) {
            printf("(%i,%i)", search_stack.data[i].ngon, search_stack.data[i].rotation);
        }
        boundary_write(boundary);
        printf("\n");
//#endif //NDEBUG
        search_stack_entry_t* entry = search_stack_top(&search_stack);
        if(boundary.bits == goal.bits && boundary.size == goal.size) {
            return 1;
        }

        
        if(already_searched != -1) {
            already_searched = 0;
            boundary_t test_boundary = start;
            boundary_t normalized_boundary = boundary_normalize(boundary);
            for(int i = 0; i < search_stack.fill; i++) {
                boundary_write(test_boundary);
                printf("\n");
                if(test_boundary.size == boundary.size) {
                    boundary_t normalized_test_boundary = boundary_normalize(test_boundary);
                    if(normalized_test_boundary.bits == normalized_boundary.bits) {
                        already_searched = 1;
                        break;
                    }
                }
                test_boundary = boundary_rotl(test_boundary, search_stack.data[i].rotation);
                boundary_t new_test_boundary = boundary_remove(test_boundary, search_stack.data[i].ngon);
                assert(new_test_boundary.size != 0);
                test_boundary = new_test_boundary;
            }
        }
        already_searched = 0;

        if(!already_searched && entry->rotation < boundary.size) {
            boundary_t new_boundary = boundary_remove(boundary, entry->ngon);
            boundary_check(new_boundary);
            if(new_boundary.size > 0 && new_boundary.size < MAX_SIZE &&
               database_contains(database, boundary_normalize(new_boundary))) {
                boundary = new_boundary;
                search_stack_push(&search_stack, (search_stack_entry_t){.ngon = small_ngon, .rotation = 0});
            } else {
                entry->rotation++;
                boundary = boundary_rotl(boundary, 1);
            }
        } else if(!already_searched && entry->ngon == small_ngon) {
            entry->rotation = 0;
            entry->ngon = large_ngon;
        } else {
            search_stack_pop(&search_stack);
            if(search_stack_is_empty(&search_stack)) {
                return 0;
            } else {
                entry = search_stack_top(&search_stack);
                boundary = boundary_insert(boundary, entry->ngon);

                entry->rotation++;
                boundary = boundary_rotl(boundary, 1);
            }
        }
    }
}

b32 search_backward_towards_database(database_t database, boundary_t start, int start_size) {
    boundary_t boundary = start;
    search_stack_do_empty(&search_stack);
    search_stack_push(&search_stack, (search_stack_entry_t){.ngon = small_ngon, .rotation = 0});
    while(1) {
#ifndef NDEBUG
        for(int i = 0; i < search_stack.fill; i++) {
            printf("(%i,%i)", search_stack.data[i].ngon, search_stack.data[i].rotation);
        }
        boundary_write(boundary);
        printf("\n");
#endif //NDEBUG
        search_stack_entry_t* entry = search_stack_top(&search_stack);
        
        if(boundary.size < MAX_SIZE && database_contains(database, boundary_normalize(boundary))) {
            return 1;
        }
        
        if(entry->rotation < boundary.size) {
            boundary_t new_boundary = boundary_remove(boundary, entry->ngon);
            if(new_boundary.size > MAX_SIZE && new_boundary.size < MAX_SEARCH_SIZE) {
                boundary = new_boundary;
                search_stack_push(&search_stack, (search_stack_entry_t){.ngon = small_ngon, .rotation = 0});
            } else {
                entry->rotation++;
                boundary = boundary_rotl(boundary, 1);
            }
        } else if(entry->ngon == small_ngon) {
            entry->rotation = 0;
            entry->ngon = large_ngon;
        } else {
            search_stack_pop(&search_stack);
            if(search_stack_is_empty(&search_stack)) {
                return 0;
            } else {
                entry = search_stack_top(&search_stack);
                boundary = boundary_insert(boundary, entry->ngon);
                entry->rotation++;
                boundary = boundary_rotl(boundary, 1);
            }
        }
    }
}



void* working_thread_add(void* thread_manager_ptr) {
    thread_manager_t* thread_manager = (thread_manager_t*)thread_manager_ptr;
    pthread_t self_id = pthread_self();
    ulong thread_number = -1;
    for(int i = 0; i < NUM_THREADS; i++) {
        if(thread_manager->thread_data[i].thread_id == self_id) {
            thread_number = i;
            break;
        }
    }
    assert(thread_number != -1);
    thread_data_t* data = &thread_manager->thread_data[thread_number];

    while(1) {
        if(thread_manager->number_of_active_threads < NUM_THREADS && data->state == THREAD_STATE_CAN_SHARE) {
            thread_try_sharing(thread_manager, thread_number);
        }
        
        int has_no_items = 1;
        for(int size = 1; size < MAX_SIZE; size++) {
            if(!queue_is_empty(&data->queues[size])) {
                boundary_t boundary = {
                    .bits = queue_dequeue(&data->queues[size]),
                    .size = size
                };
                boundary_check(boundary);
                if(!database_contains(thread_manager->database, boundary)) {
#ifndef NDEBUG
                    printf("Thread %li found ", thread_number);
                    boundary_write(boundary);
                    printf("\n");
#endif
                    for(int j = 0; j < size; j++) {
                        boundary_t new_boundary = boundary_insert(boundary, small_ngon);
                        if(new_boundary.size != 0 && new_boundary.size < MAX_SIZE) {
                            queue_enqueue(&data->queues[new_boundary.size], boundary_normalize(new_boundary).bits);
                        }
                        new_boundary = boundary_insert(boundary, large_ngon);
                        if(new_boundary.size != 0 && new_boundary.size < MAX_SIZE) {
                            queue_enqueue(&data->queues[new_boundary.size], boundary_normalize(new_boundary).bits);
                        }
                        boundary = boundary_rotl(boundary, 1);
                    }
                    database_add(thread_manager->database, boundary);
                }
                has_no_items = 0;
                if(data->queues[size].head_page != data->queues[size].tail_page) data->state = THREAD_STATE_CAN_SHARE;
                break;
            }
            
        }
        
        if(has_no_items) {
            thread_wait(thread_manager, thread_number);
        }
    };
}


int main(int argc, char* argv[]) {
    if(argc == 3) {
        small_ngon = atoi(argv[1]);
        large_ngon = atoi(argv[2]);
    };

    database_t monogon_database;
    database_t hexagon_database;

    database_init(&monogon_database);
    database_init(&hexagon_database);

    thread_manager_t monogon_thread_manager;   
    thread_manager_t hexagon_thread_manager;   

    thread_manager_init(&monogon_thread_manager, monogon_database);
    thread_manager_init(&hexagon_thread_manager, hexagon_database);

    queue_enqueue(&monogon_thread_manager.thread_data[0].queues[1], 0); // insert monogon
    queue_enqueue(&hexagon_thread_manager.thread_data[0].queues[6], 0); // insert hexagon

    for(long thread_number = 0; thread_number < NUM_THREADS; thread_number++) {
        pthread_create(&monogon_thread_manager.thread_data[thread_number].thread_id, NULL,
                       working_thread_add, (void*)&monogon_thread_manager);
    }
    pthread_mutex_unlock(&monogon_thread_manager.mutex);

    for(int thread_number = 0; thread_number < NUM_THREADS; thread_number++) {
        printf("Waiting for thread %i...\n", thread_number);
        pthread_join(monogon_thread_manager.thread_data[thread_number].thread_id, NULL);
        printf("Waiting for thread %i successfully!\n", thread_number);
    }

    for(long thread_number = 0; thread_number < NUM_THREADS; thread_number++) {
        pthread_create(&hexagon_thread_manager.thread_data[thread_number].thread_id, NULL,
                       working_thread_add, (void*)&hexagon_thread_manager);
    }
    pthread_mutex_unlock(&hexagon_thread_manager.mutex);
    for(int thread_number = 0; thread_number < NUM_THREADS; thread_number++) {
        printf("Waiting for thread %i...\n", thread_number);
        pthread_join(hexagon_thread_manager.thread_data[thread_number].thread_id, NULL);
        printf("Waiting for thread %i successfully!\n", thread_number);
    }
    
    pool_t mouse_pool;
    pool_init(&mouse_pool);
    boundary_t* mouse_data = (boundary_t*)mouse_pool.data;

    search_stack_init(&search_stack, 64);

    
    boundary_t boundary;
    for(boundary.size = 1; boundary.size * 6 < MAX_SIZE; boundary.size++) {
        for(boundary.bits = 0; boundary.bits < (1 << boundary.size); boundary.bits++) {
            if(database_contains(monogon_database, boundary)) {
                for(int k = 0; k < boundary.size; k++) {
                    if(boundary_is_mouse(boundary)) {
                        boundary_t* new_mouse = (boundary_t*)pool_alloc(&mouse_pool, sizeof(boundary_t));
                        *new_mouse = boundary;
                        printf("Mouse ");
                        boundary_write(boundary);
                        printf("\n");
                    }
                    boundary = boundary_rotl(boundary, 1);
                }
            }
        }
    }

    for(int i = 0; i < mouse_pool.length; i++) {
        boundary_t boundary = mouse_data[i];
        if(boundary.size * 6 < MAX_SIZE) {
            
            boundary_t boundary_6_expanded = boundary_unfold(boundary, 6);
            boundary_t hexagon = { .bits = 0, .size = 6 };
            b32 res = search_backward_in_database(hexagon_database, boundary_6_expanded, hexagon);
            if(res) {
                printf("Real success!!\n");
                getchar();
            }
        }
    }
    
    database_deinit(&monogon_database);
    database_deinit(&hexagon_database);
    
    search_stack_deinit(&search_stack);
    
}
