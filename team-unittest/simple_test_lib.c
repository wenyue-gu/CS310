#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/wait.h>
#include "simple_test_lib.h"

#define MAX_TESTS 10

char* (*test_funcs[MAX_TESTS])(); // array of function pointers that store
                           // all of the tests we want to run
int num_tests = 0;

void add_test(char* (*test_func)()) {
    if(num_tests == MAX_TESTS) {
        printf("exceeded max possible tests");
        exit(1);
    }
    test_funcs[num_tests] = test_func;
    num_tests++;
}

// void* run_test(void *test_to_run_void) {
//     setup();
//     char* (*test_func)() = test_to_run_void;
//     return test_func();
// }
void catch_alarm(int sig_num)
{
    //printf("Operation timed out. Exiting...\n\n");
    exit(2);
}


void run_all_tests() {

    int count_passed = 0;
    int count_failed = 0;
    int count_crashed = 0; // these types get added later in the lab
    int count_timed_out = 0;



    int pipeResult[num_tests][3];
    for(int i = 0; i<num_tests; i++){
        if (pipe(pipeResult[i]) == -1) {
            perror("pipe");
            return;
        }
    }

    pid_t pidlist[100];
    setup();
    int i = 0;
    while(i<num_tests){
        int pid = fork();
        //printf("pid %d\n", pid);
        pidlist[i] = (pid_t)pid;
        if (pid < 0) {
            perror("Fork failed.\n");
            return;  // nonzero means a failure result in unix
        }
        if(pid==0){
            signal(SIGALRM, catch_alarm);
            alarm(3);
            char* tresult = test_funcs[i]();
            alarm(0);

            //printf("where am i");
            if(tresult == TEST_PASSED) {
                //printf("no");
                //printf("passing");

                close(pipeResult[i][0]);
                close(pipeResult[i][1]);
                exit(0);
            } else {
                //printf("gonna fail");
                // char target[1+ strlen(tresult)];
                // //target = malloc(1 + strlen(tresult));
                // for(int i = 0; i<strlen(tresult); i++){
                //     target[i] = tresult[i];
                // }
                // //strcpy(target, tresult);
                // tresult[strlen(tresult)+1] = '\0';
                // //strcat(tresult, "\0");
                // close(pipeResult[0]);
                write(pipeResult[i][1], tresult, strlen(tresult));
                pipeResult[i][2] = strlen(tresult);
                close(pipeResult[i][1]);
                //free(target);
                exit(1);
            }
        }
        i++;
    }

    for(int i = 0; i<num_tests; i++){
        int status;
        //printf("%d\n", pidlist[i]);
        if(waitpid(pidlist[i],&status,0)==-1){
            perror("wait failed");
        }
        //printf("%d %d\n", pidlist[i], status);
        //wait(&status);
        if(!WIFEXITED(status)){
            printf("Test Crashed\n");
            count_crashed++;
        }
        else if(WEXITSTATUS(status)==0){
            printf("Test Passed\n");
            count_passed++;
        }
        else if(WEXITSTATUS(status)==1){
            printf("Test Failed: ");
            char buf[100];

            //printf("gonna fail");
            //read(pipeResult[0], &buf, 100);
            //printf("%s\n",buf);
            //printf("%s \n",pipeResult[i]);
            close(pipeResult[i][1]);
            //bool cond = false;
            //if(read(pipeResult[0], &buf, 1)>0) cond = true;
            // while (cond){
            //     //printf("0");
            //     //if(buf==NULL) break;
            //     //printf("%s",buf);
            //     write(STDOUT_FILENO, &buf, 1);
            //     if(read(pipeResult[0], &buf, 1)<=0){
            //         cond = false;
            //         printf("\n");
            //     }
            // }
            read(pipeResult[i][0], &buf, 100);
            printf("%s\n", buf);
            //printf("done");
            // while(read(pipeResult[i][0], &buf, 1)>0){
            //     write(STDOUT_FILENO, &buf, 1);
            // }
            count_failed++;
        }
        else if(WEXITSTATUS(status)==2){
            printf("Test Timed Out\n");
            count_timed_out++;
        }
    }



    printf("%s report: passed %d failed %d crashed %d timed out %d\n",
    TESTER_NAME, count_passed, count_failed, count_crashed, count_timed_out);
    

    //pthread_t tests[100];
    
    // int pid = fork();
    // if (pid < 0) {
    //     perror("Fork failed.\n");
    //     return;  // nonzero means a failure result in unix
    // }
    // if(pid==0){
    //     printf("child");
    //     for(int i = 0; i < num_tests; i++) {
    //         // if(pthread_create(&tests[i], NULL, &run_test, test_funcs[i])) {

    //         //     printf("Error creating thread\n");
    //         //     exit(2);

    //         // }
    //         int pid = fork();
    //         if (pid < 0) {
    //             perror("Fork failed.\n");
    //             return;  // nonzero means a failure result in unix
    //         }
    //         if(pid==0){
    //             //printf("child of child");
    //             setup();
    //             //char* (*test_func)() =test_funcs[i];
    //             test_funcs[i]();
    //             wait(1);
    //             exit(3);
    //         }

    //     }
    //     exit(3);
    // }
    // for(int i = 0; i < num_tests; i++) {
    //     char* result = NULL;
    //     if(pthread_join(tests[i],(void**) &result)) {
    //         printf("Error joining thread\n");
    //         exit(2);
    //     }
    //     if(result == TEST_PASSED) {
    //         printf("Test Passed\n");
    //         count_passed++;
    //     } else {
    //         printf("Test Failed: %s\n",result);
    //         count_failed++;
    //     }
    // }


    // printf("parent");
    // int status;
    // wait(&status);
    // printf("%s report: passed %d failed %d crashed %d timed out %d\n",
    //        TESTER_NAME, count_passed, count_failed, count_crashed, count_timed_out);
        
}

