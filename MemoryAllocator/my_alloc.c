#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <errno.h>
#include <limits.h>

// defining header:
typedef struct header{
    int size;
    int magic;
} header;

// defining free list
typedef struct free_list{
    int size;
    struct free_list *next;
}free_list;

#define hsize sizeof(header) // hsize - size of header which is 8 bytes in my case
#define fsize sizeof(free_list) // fsize - size of free list node which is 16 in my case
int *n_blocks; // to maintain the count of number of alloacated blocks

void *new_map; // new_map - it will take the pointer given by mmmap i.e starting address of assigned 4KB page
free_list *flist_head; // free list head
free_list *prev; // maintain a gloabal variable previous of the curr free list, will be used in upadating free list

// find_free_chunk(int size) - it iterated for the free_list and move to the next free chunk 
// until it found one which have size greater than or equal to the required size and return it
// First-Fit implementation
free_list* find_free_chunk(int size);

// this fuction update the free list during spliting of chunk
void update_free_list(void *ptr, int size, int total_size, free_list *next);

// split_chunk(void *s_ptr, int total, int req) - takes the the starting pointer, total size of the chunk and required size
// and if the remaining size is not enough that is less than free list size then it simply returns otherwise if it is possible
// to split, it will create a new header at the required place and update the free list accordingly
void split_chunk(void *s_ptr, int total, int req);

// add_contd_chunk(void *ptr) - this fuction is required to add the continous chunk which are free (which happen afer
// calling my_free() function), it takes the current header and make a free list there and then check where the address
// of this newly created free list lies that is it's prev free list address and next free list address
// and check if coalescing can be done or not if possible it is done accordingly
void add_contd_chunk(void *ptr);

// my_init() - to assign 4KB page with which we are supposed to work
int my_init(){
	new_map = mmap(NULL, 4096, PROT_READ|PROT_WRITE,
            MAP_ANON|MAP_PRIVATE, -1, 0);
    if(new_map == MAP_FAILED){
        //printf("MAP_FAILED");
        return -1;
    }
    n_blocks = new_map;
    *n_blocks = 0;
    //Creating the head of free list
    flist_head = (free_list *)(new_map+sizeof(int));
    flist_head->size = 4096 - sizeof(int);
    flist_head->next = NULL;
    return 0;
}

// my_alloc(int size) - it assigns the requested size if possible otherwise return error
void* my_alloc(int size){
    //checking the initial conditions that size should be greater than 0 and should be multiple of 8
    if(size <= 0 || size%8 != 0) return NULL;
    int req_size = size + hsize; // required size = header size + requested size
    free_list *free_chunk = find_free_chunk(req_size); // finding the free chunk where requested size can be allocated
    // if free chunk is found allocate the required size and split accordingly as explained in split function
    if(free_chunk){
        (*n_blocks)++;
        void *add = ((void *)free_chunk);
        split_chunk(add,((free_list *)add)->size,req_size);
        ((header *)add)->magic = 1234567;
        return ((void*)add+hsize);
    }
    return NULL;
}

free_list* find_free_chunk(int size){
    prev = NULL;
    free_list *curr = flist_head;
    while(curr){
        if(curr->size == size){
            return curr;
        }
        if(curr->size >= size+fsize){
            return curr;
        }
        prev = curr;
        curr = curr->next;
    }
    return NULL;
}

void update_free_list(void *ptr, int size, int total_size, free_list *next){
    free_list *curr = (free_list *)(ptr + size);
    curr->size = total_size-size;
    curr->next = next;
    if(prev){
        prev->next = curr;
    }
    if(ptr == (void *)flist_head){
        flist_head = curr;
    }
}

void split_chunk(void *s_ptr, int total, int req){
    int rem_size = total - req;
    free_list *next = ((free_list *)s_ptr)->next;
    header *head = (header *)s_ptr;
    head->size = req - hsize;
    if(rem_size == 0){
        if(prev){
            prev->next = next;
        }
        if(s_ptr == (void *)flist_head){
            flist_head = next;
        }
        return;
    }
    update_free_list(s_ptr,req,total,next);
}

// my_free(void *ptr) - free the space allocated to the ptr and add the continous free chunk if any as explained in add_contd_chunk function
void my_free(void *ptr){
    if(!ptr) return; // NULL pointer
    void *s_add = ptr - hsize; // address of the correspondig header
    if(((header*)s_add)->magic != 1234567) return;
    (*n_blocks)--;
    ((header*)s_add)->magic = 0;
    add_contd_chunk(s_add);
}

int cond_check(void *left, void* right, int leftsize){
    if(left+leftsize == right)
    return 1;
    return 0;
}

void add_contd_chunk(void *ptr){
    int size = ((header*)ptr)->size+hsize;
    free_list *itr = flist_head;
    if((void *)itr > ptr){
        int var = cond_check(ptr,itr,size);
        if(var == 1){
            ((free_list*)ptr)->next = itr->next;
            ((free_list*)ptr)->size = itr->size+size;
            flist_head = (free_list *)ptr; 
            return;
        }
        else{
            ((free_list*)ptr)->next = itr;
            ((free_list*)ptr)->size = size;
            flist_head = (free_list *)ptr;
            return;
        }
    }
    while (itr)
        {
            if(itr->next){
                if(ptr < (void *)itr->next && ptr > (void *)itr){
                    int var1 = cond_check(ptr,itr->next,size);
                    if(var1==1){
                        ((free_list*)ptr)->next = itr->next->next;
                        ((free_list*)ptr)->size = itr->next->size+size;
                    }
                    else{
                        ((free_list*)ptr)->next = itr->next;
                        ((free_list*)ptr)->size = size;
                    }
                    int var2 = cond_check(itr,ptr,itr->size);
                    if(var2 == 1){
                        itr->next = ((free_list*)ptr)->next;
                        itr->size += ((free_list*)ptr)->size;
                    }
                    else{
                        itr->next = (free_list *)ptr;
                    }
                    return;
                }
            }
            else{
                int var3 = cond_check(itr,ptr,itr->size);
                if(var3 == 1){
                    itr->next = NULL;
                    itr->size += size;
                }
                else{
                    itr->next = (free_list *)ptr;
                    ((free_list *)ptr)->size = size;
                    ((free_list *)ptr)->next = NULL;
                }
                return;
            }
            itr = itr->next;
        }
}

// my_clean() - returning memory back to OS using munnmap system call
void my_clean(){
    int temp = munmap(new_map, 4096);
    if(temp != 0){
        //printf("UNMAP_FAILED");
        return;
    }
}

// my_heapinfo() - give the information about current state of our heap by iterating over all the headers and getting the required info
void my_heapinfo(){
	int a = 0, b = 0, c = 0, d = *n_blocks, e = 0, f = 0;
    a = 4096 - sizeof(int); // MAX_SIZE
    e = 4096;
    f = 0;
    free_list* curr = flist_head;
    while(curr){
        int size = curr->size - fsize;
        c += size;
        if(e > size) e = size;
        if(f < size) f = size;
        curr = curr->next;
    }
    b = a - c;
    if(e == 4096){
    	e = 0;
    }
	// Do not edit below output format
	printf("=== Heap Info ================\n");
	printf("Max Size: %d\n", a);
	printf("Current Size: %d\n", b);
	printf("Free Memory: %d\n", c);
	printf("Blocks allocated: %d\n", d);
	printf("Smallest available chunk: %d\n", e);
	printf("Largest available chunk: %d\n", f);
	printf("==============================\n");
	// Do not edit above output format
	return;
}
