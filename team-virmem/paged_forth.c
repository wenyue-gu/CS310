#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/mman.h>
#include <unistd.h>
#include "forth/forth_embed.h"
#include "paged_forth.h"

int current_test_case;

void page_map_callback(void* invalid_pointer, int page_mapped, int page_unmapped) {
    printf("%s%d Mapped page %d for address %p.", PRINT_PREFIX, current_test_case, page_mapped, invalid_pointer);
    if(page_unmapped != NO_UNMAP) {
        printf("  Unmapped %d.", page_unmapped);
    }
    printf("\n");
}

/*
  This test doesn't actually run forth at all, it just expictly invokes pointers to see if your mapping works
 */
void test1and3(int max_pages) {

    struct forth_data forth;
    // note that this initialize implicitly gets page 19
    initialize_forth(&forth, max_pages);

    char* region = (char*) STACKHEAP_MEM_START;
    // sanity check for unmapping.  Set a value in page 19
    region[getpagesize()*NUM_PAGES - getpagesize()] = 77;
    
    // get page 0
    region[0] = 0;
    // get page 1
    region[getpagesize()] = 0;
    // don't actually get page 2, but get close
    region[getpagesize()*2 - 1] = 0;
    // get the 2nd to last page
    region[getpagesize()*NUM_PAGES - getpagesize() - 1] = 0;

    // get the last page again (will print something if its been unmaped)
    int data = region[getpagesize()*NUM_PAGES - getpagesize()];
    if(data != 77) {
        printf("ERROR!  When page 19 was remapped, it didn't have the right data\n");
    }

    
}

void test2and4(int max_pages) {

    struct forth_data forth;
    initialize_forth(&forth, max_pages);
    char output[200];

    // this code actually executes a large amount of starter forth
    // code in jonesforth.f.  If you screwed something up about
    // memory, it's likely to fail here.
    load_starter_forth_at_path(&forth, "forth/jonesforth.f");

    // now we can set the input to our own little forth program
    // (as a string)
    int fresult = f_run(&forth,
                        " : USESTACK BEGIN DUP 1- DUP 0= UNTIL ; " // function that puts numbers 0 to n on the stack
                        " : DROPUNTIL BEGIN DUP ROT = UNTIL ; " // funtion that pulls numbers off the stack till it finds target
                        " 5000 USESTACK " // 5000 integers on the stack
                        " 2500 DROPUNTIL " // pull half off
                        " 1000 USESTACK " // then add some more back
                        " 4999 DROPUNTIL " // pull all but 2 off
                        " . . " // 4999 and 5000 should be the only ones remaining, print them out
                        " .\" finished successfully \" " // print some text
                        ,
                        output,
                        sizeof(output));
    
    if(fresult != FCONTINUE_INPUT_DONE) {
        printf("forth did not finish executing sucessfully %d\n", fresult);
        exit(4);
    }
    printf("OUTPUT: %s\n", output);

    
}

int main(int argc, char** argv) {

    if(argc == 1) {
        printf("please enter a test case number\n");
        exit(1);
    }
    current_test_case = atoi(argv[1]);
    switch(current_test_case) {
    case 1:
        test1and3(NUM_PAGES);
        break;
    case 2:
        test2and4(NUM_PAGES);
        break;
    case 3:
        test1and3(3);
        break;
    case 4:
        test2and4(3);
        break;

    default:
        printf("unknown test case\n");
    }
    printf("done\n");
    return 0;
}
