#include <stdio.h>
#include <unistd.h>
#include "simple_test_lib.h"

#define DATA_SIZE 100
#define INITIAL_VALUE 77

int data[DATA_SIZE][DATA_SIZE]; // shared data that the tests use

// this setup function should run before each test
void setup() {
    
}


char* test4() {

    printf("starting test 4\n");

    int *val = NULL;
    printf("data at val is %d", *val);
    
    printf("ending test 4\n");
    return TEST_PASSED;
}


int main() {
    add_test(test4);
    run_all_tests();
    return 0;
}
