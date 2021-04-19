#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/mman.h>
#include <unistd.h>
#include "forth/forth_embed.h"
#include "paged_forth.h"

void* stackheap;
int max_pages;

typedef struct record {
  int priority;
  int fd;
} rec_t;

rec_t array[NUM_PAGES];

static void handler(int sig, siginfo_t *si, void *unused)
{
    void* fault_address = si->si_addr;

    //printf("in handler with invalid address %p\n", fault_address);
    int distance = (void*) fault_address - (void*) STACKHEAP_MEM_START;
    if(distance < 0 || distance > getpagesize() * NUM_PAGES) {
        printf("address not within expected region!\n");
        exit(2);
    }

    int pagenum = distance/getpagesize();
    void* address = (void*) STACKHEAP_MEM_START + pagenum * getpagesize();

    int index = -1;
    int i =0;
    for(i=0; i<NUM_PAGES;i++){
        if(array[i].priority==0) continue; //not mapped
        else{
            array[i].priority=array[i].priority-1;
            if(array[i].priority==0){
                //unmap the thing;
                void* uad = (void*) STACKHEAP_MEM_START + i * getpagesize();
                int munmap_result = munmap(uad, getpagesize());
                if(munmap_result < 0) {
                    perror("munmap failed");
                    exit(6);
                }
                index = i;

            }
        }
    }
    array[pagenum].priority = max_pages;




    if(array[pagenum].fd==-1){
        char filename [100];
        snprintf(filename, 100, "page_%d.dat", pagenum);

        int fd = open(filename, O_RDWR | O_CREAT, S_IRWXU);
        if(fd < 0) {
            perror("error loading linked file");
            exit(25);
        }
        char data = '\0';
        lseek(fd, getpagesize() - 1, SEEK_SET);
        write(fd, &data, 1);
        lseek(fd, 0, SEEK_SET);
        array[pagenum].fd = fd;
    }

    char* result = mmap(address,
                        getpagesize(),
                        PROT_READ | PROT_WRITE | PROT_EXEC,
                        MAP_FIXED | MAP_SHARED,
                        array[pagenum].fd, 0);


    // in your code you'll have to compute a particular page start and
    // map that, but in this example we can just map the same page
    // start all the time
    //printf("mapping page starting at %p\n", address);
    // page_map_callback(fault_address, pagenum, NO_UNMAP);
    // void* result = mmap((void*) address,
    //                     getpagesize(),
    //                     PROT_READ | PROT_WRITE | PROT_EXEC,
    //                     MAP_FIXED | MAP_SHARED | MAP_ANONYMOUS,
    //                     -1,
    //                     0);
    if(index==-1){
        page_map_callback(fault_address, pagenum, NO_UNMAP);
    }
    else{
        page_map_callback(fault_address, pagenum, index);
    }
    if(result == MAP_FAILED) {
        perror("map failed");
        exit(1);
    }
}


void initialize_forth(struct forth_data *forth, int max_pages_input) {


    static char stack[SIGSTKSZ];
    
    stack_t ss = {
                  .ss_size = SIGSTKSZ,
                  .ss_sp = stack,
    };
    
    sigaltstack(&ss, NULL);

    struct sigaction sa;

    // SIGINFO tells sigaction that the handler is expecting extra parameters
    // ONSTACK tells sigaction our signal handler should use the alternate stack
    sa.sa_flags = SA_SIGINFO | SA_ONSTACK;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = handler;

    //this is the more modern equalivant of signal, but with a few
    //more options
    if (sigaction(SIGSEGV, &sa, NULL) == -1) {
        perror("error installing handler");
        exit(3);
    }

    int i = 0;
    for(i = 0; i<NUM_PAGES; i++){
        array[i].fd = -1;
    }

    max_pages = max_pages_input;
    
    // the return stack is a forth-specific data structure if we
    // wanted to, we could give it an expanding memory segment like we
    // do for the stack/heap but I opted to keep things simple
    //
    // note this static is really important
    static char returnstack[1024];

    // because this might be called multiple times, we unmap the
    // region
    int result = munmap((void*) STACKHEAP_MEM_START, getpagesize()*NUM_PAGES);
    if(result == -1) {
        perror("error unmapping");
        exit(1);
    }

    
    int stackheap_size = getpagesize() * NUM_PAGES;

    //stackheap = mmap((void*) STACKHEAP_MEM_START, stackheap_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_PRIVATE, -1, 0);
    stackheap = (void*) STACKHEAP_MEM_START;
    
    // printf("stack at %p\n", stackheap);

    initialize_forth_data(forth,
                          returnstack + sizeof(returnstack), //beginning of returnstack
                          stackheap, //begining of heap
                          stackheap + stackheap_size);

    //exit(0);
}


