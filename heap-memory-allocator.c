#include <stdio.h>
#include <unistd.h>

#define META_DATA_SIZE sizeof(__md_block)

typedef struct data {
    struct data *next;    // pointing to the next memory block
    size_t size;          // size of the allocated memory
    int is_free;          // state of the memory
} __md_block;

__md_block *head = NULL, *tail = NULL;

void* access_free_segmented_mem(size_t size){
    __md_block* cur = head;
    while(cur != NULL){
        if(cur->is_free && cur->size >= size){ 
            cur->is_free = 0;
            cur->size = size; 
            return (void*)(cur+1); 
        } 
        cur = cur->next; 
    }
    return NULL;
}

void *mem_alloc(size_t size) {

    if(!size) { return NULL; }

    void* mem = access_free_segmented_mem(size);
    if(mem != NULL){ return mem; }
    
    __md_block *meta_data = sbrk(0);
    if(sbrk(META_DATA_SIZE + size) == (void *)-1) { return NULL; }
    
    meta_data->next = NULL;
    meta_data->size = size;
    meta_data->is_free = 0;
    
    if(head == NULL) { head = meta_data; }
    if(tail != NULL) { tail->next = meta_data; }
    tail = meta_data;

    
    return (void *)(meta_data+1);
}

void join_free_contiguous_mem(){
    __md_block* cur = head, * prev = NULL;
    while(cur->next != NULL){
       prev = cur;
       cur = cur->next; 
       if(cur->is_free && prev->is_free){
           prev->next = cur->next;
           cur->next = NULL;
           prev->size += cur->size + META_DATA_SIZE;
       } 
    } 
}

void mem_free(void *address) {
    __md_block *memory = ((__md_block *)address) - 1;
    memory->is_free = 1;

    if(tail->next == NULL){ sbrk(0 - META_DATA_SIZE - memory->size); }
    
    if(tail == head){ head = NULL; }
    tail = NULL;

    join_free_contiguous_mem();
}


int main() {
    return 0;
}
