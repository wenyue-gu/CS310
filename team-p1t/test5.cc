#include <stdlib.h>
#include <iostream>
#include "thread.h"
#include <assert.h>

using namespace std;

void methodA(void *a) {
	cout << "A";
	thread_wait(2,7);
	cout << "B";
	thread_lock(7);
	cout << "C";
	thread_signal(2, 7);
	cout << "D";
	thread_yield();
	cout << "E";
	thread_unlock(2);
	cout << "F";
	thread_wait(2, 3);
	cout << "G";
	thread_lock(7);
	cout << "H";
	thread_yield();
	cout << "I";
	thread_lock(7);
	cout << "J";
	thread_unlock(7);
	cout << "K";
	thread_unlock(3);
	cout << "L";
	thread_wait(3, 2);
	cout << "M";
	thread_yield();
	cout << "Y";
	thread_lock(2);
	cout << "Z";
	thread_unlock(2);
}

void methodB(void *a) {
	cout << "N";
	thread_lock(7);
	cout << "O";
	thread_wait(2, 7);
	cout << "P";
	thread_broadcast(2, 7);
	cout << "Q";
	thread_unlock(7);
	cout << "R";
	thread_lock(2);
	cout << "S";
	thread_lock(7);
	cout << "T";
	thread_wait(2, 7);
	cout << "U";
	thread_yield();
	cout << "V";
	thread_yield();
	cout << "X";
} 

void methodC(void *a) {
	cout << "R";
	thread_yield();
	cout << "x";
	thread_yield();
	cout << "V";
	thread_yield();
	cout << "r";
}

void parent(void* a) {
  thread_create( (thread_startfunc_t) methodA, a);
  thread_create( (thread_startfunc_t) methodB, a);
  thread_create( (thread_startfunc_t) methodC, a);
}

int main() {
  if (thread_libinit( (thread_startfunc_t) parent, (void *) 200)) {
    cout << "thread_libinit failed\n";
    exit(2);
  }
}