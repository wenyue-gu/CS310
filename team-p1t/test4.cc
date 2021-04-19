#include <stdlib.h>
#include <iostream>
#include "thread.h"
#include <assert.h>

using namespace std;

void red(void *a) {
	cout << "1";
	thread_wait(1,2);
	cout << "2";
}

void black(void *b) {
	thread_lock(1);
	cout << "3";
	thread_unlock(1);
} 

void parent(void* a) {
  thread_create( (thread_startfunc_t) red, a);
  thread_create( (thread_startfunc_t) black, a);
}

int main() {
  if (thread_libinit( (thread_startfunc_t) parent, (void *) 100)) {
    cout << "thread_libinit failed\n";
    exit(1);
  }
}