#include <stdio.h>
#include <unistd.h>
#include "simple_test_lib.h"

#define DATA_SIZE 100
#define INITIAL_VALUE 77

int data[DATA_SIZE][DATA_SIZE]; // shared data that the tests use

// this setup function should run before each test
void setup() {
}


char* test1() {

    return "failure message\n" TESTER_NAME " test1 failure\n";
}

char* test2() {

    return "failure message\n" TESTER_NAME " test2 failure\n";
}

int main() {
    add_test(test1);
    add_test(test2);
    run_all_tests();
    return 0;
}
