#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <atomic>
#include "queue.h"
#include "CuTest.h"

#define VALUES_PER_THREAD 2000
#define NUMBER_OF_THREADS 1000

volatile std::atomic<int> dequeued_values_counter[VALUES_PER_THREAD];

queue_t global_queue;

void insert_values_in_queue() {
    for(int i = 0; i < VALUES_PER_THREAD; i++) {
        enqueue(&global_queue, i);
    }
}

void remove_values_from_queues() {
    for(int i = 0; i < VALUES_PER_THREAD; i++) {
        int removed_value;
        do {
            removed_value = dequeue(&global_queue);   
        } while(removed_value == DEQUEUE_FAIL);
        atomic_fetch_add(&dequeued_values_counter[removed_value], 1);
    }
}

void remove_all_values_from_queues() {
    while(true) {
        int removed_value = dequeue(&global_queue);
        if(removed_value == DEQUEUE_FAIL)
            break;
        dequeued_values_counter[removed_value]++;
    }
}

void test_enqueue_and_dequeue_nonparallel(CuTest *tc) {
    initialize(&global_queue);
    for(int i = 0; i < VALUES_PER_THREAD; i++) {
        dequeued_values_counter[i] = 0;
    }
    pthread_t enqueue_threads[NUMBER_OF_THREADS], dequeue_threads[NUMBER_OF_THREADS];
    insert_values_in_queue();
    insert_values_in_queue();
    remove_values_from_queues();
    remove_values_from_queues();
    
    
    destroy(&global_queue);

    int missing_values = 0;
    for(int i = 0; i < VALUES_PER_THREAD; i++) {
        CuAssertIntEquals(tc, 2, dequeued_values_counter[i]);
    }

}


void test_enqueue_and_dequeue_parallel(CuTest *tc) {

    initialize(&global_queue);
    for(int i = 0; i < VALUES_PER_THREAD; i++) {
        dequeued_values_counter[i] = 0;
    }
    pthread_t enqueue_threads[NUMBER_OF_THREADS], dequeue_threads[NUMBER_OF_THREADS];
    for(int i = 0; i < NUMBER_OF_THREADS; i++) {
        pthread_create(&enqueue_threads[i], NULL, (void * (*)(void *)) insert_values_in_queue, NULL);

        pthread_create(&dequeue_threads[i], NULL, (void * (*)(void *)) remove_values_from_queues, NULL);

    }
    for(int i = 0; i < NUMBER_OF_THREADS; i++) {
        pthread_join(enqueue_threads[i], NULL);
        pthread_join(dequeue_threads[i], NULL);
    }

    destroy(&global_queue);

    int missing_values = 0;
    for(int i = 0; i < VALUES_PER_THREAD; i++) {
        CuAssertIntEquals(tc, NUMBER_OF_THREADS, dequeued_values_counter[i]);
    }
}

void test_enqueue_parallel(CuTest *tc) {

    initialize(&global_queue);
    for(int i = 0; i < VALUES_PER_THREAD; i++) {
        dequeued_values_counter[i] = 0;
    }
    pthread_t enqueue_threads[NUMBER_OF_THREADS], dequeue_threads[NUMBER_OF_THREADS];
    for(int i = 0; i < NUMBER_OF_THREADS; i++) {
        pthread_create(&enqueue_threads[i], NULL, (void * (*)(void *)) insert_values_in_queue, NULL);

    }
    for(int i = 0; i < NUMBER_OF_THREADS; i++) {
        pthread_join(enqueue_threads[i], NULL);
    }

    remove_all_values_from_queues();
    
    destroy(&global_queue);

    int missing_values = 0;
    for(int i = 0; i < VALUES_PER_THREAD; i++) {
        CuAssertIntEquals(tc, NUMBER_OF_THREADS, dequeued_values_counter[i]);
    }
}


void test_dequeue_parallel(CuTest *tc) {

    initialize(&global_queue);
    for(int i = 0; i < VALUES_PER_THREAD; i++) {
        dequeued_values_counter[i] = 0;
        insert_values_in_queue();
    }
    pthread_t dequeue_threads[NUMBER_OF_THREADS];

    for(int i = 0; i < NUMBER_OF_THREADS; i++) {
        pthread_create(&dequeue_threads[i], NULL, (void * (*)(void *)) remove_values_from_queues, NULL);

    }
    
    for(int i = 0; i < NUMBER_OF_THREADS; i++) {
        pthread_join(dequeue_threads[i], NULL);
    }

    destroy(&global_queue);

    int missing_values = 0;
    for(int i = 0; i < VALUES_PER_THREAD; i++) {
        CuAssertIntEquals(tc, NUMBER_OF_THREADS, dequeued_values_counter[i]);
    }
}



int main() {
    CuString *output = CuStringNew();
    CuSuite* suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, test_enqueue_and_dequeue_nonparallel); 
    SUITE_ADD_TEST(suite, test_dequeue_parallel);
    SUITE_ADD_TEST(suite, test_enqueue_parallel); 
    SUITE_ADD_TEST(suite, test_enqueue_and_dequeue_parallel); 

    CuSuiteRun(suite);
    CuSuiteSummary(suite, output);
    CuSuiteDetails(suite, output);
    printf("%s\n", output->buffer);
    CuStringDelete(output);
    CuSuiteDelete(suite);

}
