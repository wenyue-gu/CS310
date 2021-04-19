// if the region requested is already mapped, things fail
// so we want address that won't get used as the program
// starts up
#define STACKHEAP_MEM_START 0xf9f8c000

// the number of memory pages we will allocate to an instance of forth
#define NUM_PAGES 20


/*

This is the function where you setup your forth and register the
signal handler you'd like to use for callbacks.

max_pages is the max number of pages to allow to be loaded into memory
at once.  Not used for the first part (set to NUM_PAGES or basically
no limit) but used in the second.

 */
void initialize_forth(struct forth_data *forth, int max_pages);

#define PRINT_PREFIX "PMC-TC"

#define NO_UNMAP -1

/*

This is a function to call to indicate the result of a page mapping.  It'll print the intformation for your convenience and its also used in test cases.

 */
void page_map_callback(void* invalid_pointer, int page_mapped, int page_unmapped);
