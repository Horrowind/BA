void* allocate_page(page_allocator_t* page_allocator) {
    void* new_page;
    if(page_allocator->empty_list == NULL) {
	new_page = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    } else {
	new_page = page_allocator->empty_list;
	page_allocator->empty_list = *(void*)(new_page);
    }
    return new_page;
}

void* deallocate_page(page_allocator_t* page_allocator, void* page) {
    void* old_empty_list_head = page_allocator->empty_list;
    page_allocator->empty_list = page;
    (void*)(new_page) = old_empty_list_head;
}

void queue_enqueue(queue_t* queue, inttype item) {
    if(queue->head_index < QUEUE_SIZE_PER_PAGE) {
	queue->head_page.entries[queue->head_index] = item;
	queue->head_index++;
    } else {
	queue->head_page.next_page = (queue_page_t*)allocate_page(&queue->page_allocator);
	queue->head_page.next_page->next_page = NULL;
	queue->head_page = queue->head_page.next_page;
	queue->head_page.entries[0] = item;
        queue->head_index = 1;
    }
}

b32 queue_is_empty(queue_t* queue) {
    return (queue->head_page == queue->tail_page) && (queue->head_index == queue->tail_index);
}

inttype queue_dequeue(queue_t* queue) {
    inttype result;
    if(queue->tail_index < QUEUE_SIZE_PER_PAGE) {
	result = queue->tail_page.entries[queue->tail_index];
	queue->tail_index++;
    } else {
	queue_page_t* new_tail_page = queue->tail_page.next_page;
	deallocate_page(&queue_page_allocator, queue->tail_page);
	queue->tail_page = new_tail_page;
	result = queue->tail_page.entries[0];
	queue->tail_index = 1;
    }
    return result;
}
