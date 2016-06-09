#ifndef UTIL_H
#define UTIL_H

#define PAGE_SIZE 4096
#define QUEUE_SIZE_PER_PAGE ((PAGE_SIZE - sizeof(void*)) / sizeof(inttype))

typedef unsigned char u8;
typedef char i8;
typedef unsigned short u16;
typedef short i16;
typedef unsigned int u32;
typedef signed int i32;
typedef unsigned long u64;
typedef signed long i64;

typedef long boundary;

typedef struct {
    void* empty_list;
} page_allocator_t;

typedef struct queue_page {
    struct queue_page* next_page;
    inttype entries[QUEUE_SIZE_PER_PAGE];
} queue_page_t;

typedef struct {
    void* head_page;
    int head_index;
    void* tail_page;
    int tail_index;
    page_allocator_t page_allocator;
} queue_t;

void* allocate_page(page_allocator_t* page_allocator);
void* deallocate_page(page_allocator_t* page_allocator, void* page);
b32 queue_is_empty(queue_t* queue);
void queue_enqueue(queue_t* queue, inttype item);
inttype queue_dequeue(queue_t* queue);

#endif
