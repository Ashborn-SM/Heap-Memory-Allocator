#include <stdio.h>
#include <unistd.h>

typedef struct meta_data_{
    size_t size;
    unsigned int is_free;
    struct meta_data_* next;
}meta_data;

meta_data* head = NULL, * tail = NULL;

void* access_free_memory(size_t size){
    meta_data* cur = head;
    while(cur != NULL){
        if(cur->is_free && cur->size >= size){
            cur->size = size;
            cur->is_free = 0;
            return cur;
        }
        cur = cur->next;
    }
    return NULL;
}

void* c_malloc(size_t size){
    size_t total_size;
    meta_data* header = NULL;
    if(!size){
        return;
    }
    header = access_free_memory(size);
    if(header != NULL){
        return (void*)(header+1);
    }
    total_size = sizeof(meta_data) + size;
    void* block = sbrk(total_size);
    if(block == (void*) -1){
        return;
    }
    header = block;
    header->is_free = 0;
    header->size = size;
    header->next = NULL;
    if(head == NULL){
        head = header;
    }
    if(tail != NULL){
        tail->next = header;
    }
    tail = header;
    return (void*)(header+1);
}

void join_free_contiguous_block(){
    meta_data* cur = head, * prev = NULL;
    while(cur->next){
        prev = cur;
        cur = cur->next;
        if(cur->is_free && prev->is_free){
            prev->size += cur->size;
            prev->next = cur->next;

            cur->next = NULL;
            cur->is_free = 0;
            cur->size = 0;
        }
    }
}

void c_free(void* ptr){
    meta_data* cur = head, * data = (meta_data*)ptr - 1;
    void* program_break = sbrk(0);
    if((char*)(ptr)+data->size == program_break){
        if(head == tail){
            head = NULL;
            tail = NULL;
        }
        else{
            while(cur){
                if(cur->next == tail){
                    cur->next = NULL;
                    tail = cur;
                }
                cur = cur->next;
            }
        }
        sbrk(0-data->size-sizeof(meta_data));
        return;
    }
    data->is_free = 1;
    join_free_contiguous_block();
    return;
}

void c_memcpy(void* dest_ptr, void* src_ptr, size_t size){
    char* dest = dest_ptr;
    char* src = src_ptr;
    for(int i=0; i<size; i++){
        *(dest+i) = *(src+i);
    }
}

void c_memset(void* ptr, int value, size_t size){
    char* pointer = ptr;
    while(size--){
        *(pointer)++ = value; 
    }
}

void* c_calloc(size_t num, size_t size){
    if(!size){ return NULL; }
    void* new_block = c_malloc(size*num);
    if(new_block){
        c_memset(new_block, 0, size*num);
    }
    return new_block;
}

void* c_realloc(void* ptr, size_t size){
    if(!ptr || !size){ return NULL; }
    void* new_block = c_malloc(size);
    if(new_block){
        c_memcpy(new_block, ptr, ((meta_data*)ptr-1)->size);
        c_free(ptr);
    }
    return new_block;
}

void print_mem_list()
{
	meta_data *curr = head;
	printf("head = %p, tail = %p \n", (void*)head, (void*)tail);
	while(curr) {
		printf("addr = %p, size = %zu, is_free=%u, next=%p\n",
			(void*)curr, curr->size, curr->is_free, (void*)curr->next);
		curr = curr->next;
	}
}

int main(){
    int* a = c_calloc(10, sizeof(int));
    //for(int i=0; i<10; i++){ a[i] = i*i; }
    for(int i=0; i<10; i++){ printf("%i ", a[i]); }
    print_mem_list();
}