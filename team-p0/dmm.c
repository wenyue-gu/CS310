#include <stdio.h>  // needed for size_t etc.
#include <unistd.h> // needed for sbrk etc.
#include <sys/mman.h> // needed for mmap
#include <assert.h> // needed for asserts
#include "dmm.h"

/* 
 * The lab handout and code guide you to a solution with a single free list containing all free
 * blocks (and only the blocks that are free) sorted by starting address.  Every block (allocated
 * or free) has a header (type metadata_t) with list pointers, but no footers are defined.
 * That solution is "simple" but inefficient.  You can improve it using the concepts from the
 * reading.
 */

/* 
 *size_t is the return type of the sizeof operator.   size_t type is large enough to represent
 * the size of the largest possible object (equivalently, the maximum virtual address).
 */

typedef struct metadata {
  size_t size;
  struct metadata* next;
  struct metadata* prev;
  bool freed;
} metadata_t;

/*
 * Head of the freelist: pointer to the header of the first free block.
 */

static metadata_t* freelist = NULL;

void* dmalloc(size_t numbytes) {

  if(freelist == NULL) {
    if(!dmalloc_init()) {
      return NULL;
    }
  }

  assert(numbytes > 0);


  /* your code here */

  numbytes = ALIGN(numbytes);
  metadata_t* pointer = freelist; 

  while(pointer !=NULL){

    size_t size = sizeof(metadata_t) + numbytes;
    //printf("current address %p, with size %ld, freed is %d, lf size %ld\n", pointer, pointer->size, pointer->freed, size);

    if(pointer->size == (numbytes + sizeof(metadata_t)) && pointer->freed){
      pointer->freed = false;
      return (void*)(pointer +1);
    }

    if((sizeof(metadata_t) + numbytes) < pointer->size  && pointer->freed){


      if((pointer->size - size) <= sizeof(metadata_t)){
        pointer->freed = false; 
        return (void*)(pointer +1);
      }
      metadata_t* p = pointer;

      //record old data
      size_t osize = pointer->size;
      metadata_t* nex = pointer->next;

      //move pointer to next thing
      pointer = pointer+1;
      pointer = (metadata_t*) ((void*)pointer + numbytes);

      pointer->size = osize - size; 
      p->size = size; 
      p->next = pointer; 
      pointer->prev = p; 

      if(nex !=NULL){
        nex->prev = pointer; 
      }
      pointer->next = nex; 
      p->freed = false; 
      pointer->freed = true; 

      return (void*)(p +1); //Points to byte after header
    }
  pointer = pointer->next; 
  }
  return NULL;

}


void converge(metadata_t* pointer){
  if(pointer->prev!=NULL){
    if(pointer->prev->freed == true){
      size_t tsize = pointer->prev->size + pointer->size;
      pointer = pointer->prev;
      pointer->next = pointer->next->next;
      pointer->size = tsize;
      if(pointer->next!=NULL){
        pointer->next->prev = pointer;
      }
    }
  }

  if(pointer->next!=NULL){
    if(pointer->next->freed == true){
      size_t tsize = pointer->next->size + pointer->size;
      pointer->next = pointer->next->next;
      pointer->size = tsize;
      if(pointer->next!=NULL){
        pointer->next->prev = pointer;
      }
    }
  }
}


void dfree(void* ptr) {
  /* your code here */

  if(ptr==NULL) return;

  ptr = ptr - sizeof(metadata_t);
  metadata_t* pointer = freelist;
  while(pointer != NULL){
    //printf("current pointer address is %p, looking for %p \n",pointer,ptr);
    if(ptr == pointer && !pointer->freed){ 
      pointer->freed = true;
      converge(pointer);
      return;
    }
    pointer = pointer->next; 
  }
  return;
}


  // //converging
  // bool changes = true;
  // while(changes){
  //   changes = false;
  //   metadata_t* cpre = freelist;
  //   metadata_t* ctr = freelist->next;
  //   while(!ctr==NULL){
  //     //printf("here");
  //     printf("pointer one at %p, two at %p, one has size %ld\n", cpre,ctr,cpre->size);
  //     if((void*)cpre+cpre->size+sizeof(metadata_t)==(void*)ctr){
  //       changes = true;
  //       printf("can converge.\n");
  //       cpre->size = cpre->size+ctr->size+sizeof(metadata_t);
  //       cpre->next = ctr->next;

  //       printf("converged. Current size is %ld, next element is at %p\n",(cpre->size),cpre->next);
  //     }
  //     printf("cannot converge.\n");
  //     cpre = cpre->next;
  //     if(cpre==NULL) break;
  //     ctr = cpre->next;
  //   }
  // }


/*
 * Allocate heap_region slab with a suitable syscall.
 */
bool dmalloc_init() {

  size_t max_bytes = ALIGN(MAX_HEAP_SIZE);

  /*
   * Get a slab with mmap, and put it on the freelist as one large block, starting
   * with an empty header.
   */
  freelist = (metadata_t*)
     mmap(NULL, max_bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  if (freelist == (void *)-1) {
    perror("dmalloc_init: mmap failed");
    return false;
  }
  freelist->next = NULL;
  freelist->prev = NULL;
  freelist->freed = true;
  freelist->size = max_bytes-METADATA_T_ALIGNED;

  return true;
}


/* for debugging; can be turned off through -NDEBUG flag*/
/*

This code is here for reference.  It may be useful.
Warning: the NDEBUG flag also turns off assert protection.


void print_freelist(); 

#ifdef NDEBUG
	#define DEBUG(M, ...)
	#define PRINT_FREELIST print_freelist
#else
	#define DEBUG(M, ...) fprintf(stderr, "[DEBUG] %s:%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
	#define PRINT_FREELIST
#endif


void print_freelist() {
  metadata_t *freelist_head = freelist;
  while(freelist_head != NULL) {
    DEBUG("\tFreelist Size:%zd, Head:%p, Prev:%p, Next:%p\t",
	  freelist_head->size,
	  freelist_head,
	  freelist_head->prev,
	  freelist_head->next);
    freelist_head = freelist_head->next;
  }
  DEBUG("\n");
}
*/
