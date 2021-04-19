#include <stdio.h>
#include <unistd.h>
#include "simple_test_lib.h"

#define DATA_SIZE 100
#define INITIAL_VALUE 77

int data[DATA_SIZE][DATA_SIZE]; // shared data that the tests use

// this setup function should run before each test
void setup() {
}


char* test5() {

    printf("starting test 5\n");

    while(1) { } 
    
    printf("ending test 5\n");
    return TEST_PASSED;
}

int main() {
    add_test(test5); // uncomment for Step 5
    run_all_tests();
    return 0;
}
