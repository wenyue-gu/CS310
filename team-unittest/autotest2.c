#include <stdio.h>
#include <unistd.h>
#include "simple_test_lib.h"

#define DATA_SIZE 100
#define INITIAL_VALUE 77

int data[DATA_SIZE][DATA_SIZE]; // shared data that the tests use

// this setup function should run before each test
void setup() {
    printf(TESTER_NAME " Running setup\n");

    for(int i = 0; i < DATA_SIZE; i++) {
        for(int j = 0; j < DATA_SIZE; j++) {
            data[i][j] = INITIAL_VALUE;
        }
    }    
}


char* test1() {

    for(int i = 0; i < DATA_SIZE; i++) {
        for(int j = 0; j < DATA_SIZE; j++) {
            if(data[i][j] != INITIAL_VALUE) {
                printf(TESTER_NAME " FAILURE\n");
                return "failed";
            }
        }
    }

    printf(TESTER_NAME " SUCCESS\n");
    return TEST_PASSED;
}

char* test2() {

    for(int i = 0; i < DATA_SIZE; i++) {
        for(int j = 0; j < DATA_SIZE; j++) {
            if(data[i][j] != INITIAL_VALUE) {
                printf(TESTER_NAME " FAILURE\n");
                return "failed";
            }
        }
    }

    printf(TESTER_NAME " SUCCESS\n");
    return TEST_PASSED;
}

int main() {
    add_test(test1);
    add_test(test2);
    run_all_tests();
    return 0;
}
