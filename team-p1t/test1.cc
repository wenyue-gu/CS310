#include <stdlib.h>
#include <iostream>
#include "thread.h"
#include <assert.h>
using namespace std;

int c = 10;

void red(void* arg){
  if (thread_lock(1) < 0) { 
    cout << "thread lock red failed\n";
  }else{
    cout << "thread lock red successful\n";
  }

  while(c > 0){
    cout << "in red\n";
    if (thread_signal(1, 1) < 0){ 
      cout << "red signal failed\n";
    } else{
      cout << "red signal succeeded\n";
    }

    if (thread_wait(1, 1) < 0){
      cout << "red wait failed\n";
    }else{
      cout << "red waits\n";
    }

    c--;
  }

  if (thread_unlock(1) < 0) {
    cout << "thread unlock failed in red\n";
  }else{
    cout << "thread unlock successful in red\n";
  }
  
}

void black(void* arg){

  if (thread_lock(1) < 0) {
    cout << "thread lock black failed\n";
  }else{
    cout << "thread lock black successful\n";
  }

  while (c > 0){

    cout << "in black\n";
    if (thread_signal(1, 1) < 0){ 
      cout << "black signal failed\n";
    } else{
      cout << "black signal succeeded\n";
    }

    if (thread_wait(1, 1) < 0){
      cout << "black wait failed\n";
    }else{
      cout << "black waits\n";
    }
    
    c--;
  }

  if (thread_unlock(1) < 0) {
    cout << "thread unlock failed in black\n";
  }else{
    cout << "thread unlock successful in black\n";
  }

  exit(0);
  
}

void start(void* arg){
  cout << "thread enters parent\n";
  if (thread_create((thread_startfunc_t) red, (void*) 100) < 0){
    cout << "\nred create failed\n";
    exit(1);
  }else{
    cout << "\nred create created\n";
  }
  if (thread_create((thread_startfunc_t) black, (void*) 100) < 0){
    cout << "\nblack create failed\n";
    exit(1);
  }else{
    cout << "\nblack creat created\n";
  }
}


int main() {
  if (thread_libinit( (thread_startfunc_t) start, (void *) 100)) {
    cout << "thread_libinit failed\n";
    exit(1);
  }else{
    cout << "thread_libinit susceeded\n";
  }
}