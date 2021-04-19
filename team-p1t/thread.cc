#include "interrupt.h"
#include "thread.h"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>
#include <queue>
#include <ucontext.h>

using namespace std;

 
struct TCB {
  ucontext_t* ucontext; 
  bool death; 
  };

static TCB* curr;
static TCB* swi;
static queue<TCB*> readyqueue;
static bool initiated = false;

static map<unsigned int, TCB*> lockowner;
static map<unsigned int, queue<TCB*> > lockwait;
static map<pair<unsigned int, unsigned int>, queue<TCB*> > cvqueue;


static void cleanup() {
  if (curr == NULL) {
    return;
  }
  if (curr->death){
    delete (char*) curr->ucontext->uc_stack.ss_sp;
    curr->ucontext->uc_stack.ss_sp = NULL;
    curr->ucontext->uc_stack.ss_size = 0;
    curr->ucontext->uc_stack.ss_flags = 0;
    curr->ucontext->uc_link = NULL;
    delete curr->ucontext;
    delete curr;
    curr = NULL;
  }
}


static void start(thread_startfunc_t func, void *arg) {
  while (!readyqueue.empty()) {
    cleanup();
    curr = readyqueue.front();
    readyqueue.pop();

    swapcontext(swi->ucontext, curr->ucontext);
  }
  cleanup();
  cout << "Thread library exiting.\n";
  exit(0);
}

int thread_libinit(thread_startfunc_t func, void *arg) {
  if (initiated) {
    return -1;
  }
  initiated = true;

  try {
    swi = new TCB;
    swi->death = false;

    swi->ucontext = new ucontext_t;
    getcontext(swi->ucontext);
    swi->ucontext->uc_stack.ss_sp = new char [STACK_SIZE];
    swi->ucontext->uc_stack.ss_size = STACK_SIZE;
    swi->ucontext->uc_stack.ss_flags = 0;
    swi->ucontext->uc_link = NULL;
    makecontext(swi->ucontext, (void (*)()) start, 2, func, arg);
  }
  catch (bad_alloc b) {
    delete (char*) swi->ucontext->uc_stack.ss_sp;
    delete swi->ucontext;
    delete swi;
    return -1;
  }

  if (thread_create(func, arg) == -1) {
    return -1;
  }

  curr = readyqueue.front();
  readyqueue.pop();

  func(arg);
  interrupt_disable();
  curr->death = true;
  swapcontext(curr->ucontext, swi->ucontext);
}

static void trampoline(thread_startfunc_t func, void *arg) {

  interrupt_enable();
  func(arg);
  interrupt_disable();

  curr->death = true;

  swapcontext(curr->ucontext, swi->ucontext);
}

int thread_create(thread_startfunc_t func, void *arg) {
  interrupt_disable();

  if (!initiated) {
    interrupt_enable();
    return -1;
  }

  TCB* nt;
  try {
    nt = new TCB;
    nt->death = false;

    nt->ucontext = new ucontext_t;
    getcontext(nt->ucontext);
    nt->ucontext->uc_stack.ss_sp = new char [STACK_SIZE];
    nt->ucontext->uc_stack.ss_size = STACK_SIZE;
    nt->ucontext->uc_stack.ss_flags = 0;
    nt->ucontext->uc_link = NULL;
    makecontext(nt->ucontext, (void (*)())trampoline, 2, func, arg);

    readyqueue.push(nt);
  }
  catch (bad_alloc b) {
    delete (char*) nt->ucontext->uc_stack.ss_sp;
    delete nt->ucontext;
    delete nt;
    interrupt_enable();
    return -1;
  }
  interrupt_enable();
  return 0;
}

int thread_yield(void) {
  interrupt_disable();
  if (!initiated) {
    interrupt_enable();
    return -1;
  }

  readyqueue.push(curr);
  swapcontext(curr->ucontext, swi->ucontext);
  interrupt_enable();
  return 0;
}


int thread_lock(unsigned int lock){

  interrupt_disable();
  if (!initiated || lockowner[lock] == curr) {
    interrupt_enable();
    return -1;
  }

  
  if (lockwait.count(lock) == 0) {
    lockowner[lock] = NULL; 
    queue<TCB*> q1; 
    //lockwait.emplace(lock,q1);
    lockwait.insert(pair<unsigned int, queue<TCB*> >(lock, q1) );
  }
  if (lockowner[lock] != NULL) {
    lockwait[lock].push(curr); 
    swapcontext(curr->ucontext, swi->ucontext);
  } else {
    lockowner[lock] = curr;
  }

  interrupt_enable();
  return 0;
}

void unlock(unsigned int lock){
  lockowner[lock] = NULL;

  if (!lockwait[lock].empty()) {
    readyqueue.push(lockwait[lock].front());
    lockowner[lock] = lockwait[lock].front(); 
    lockwait[lock].pop(); 
  }
}


int thread_unlock(unsigned int lock){
  interrupt_disable();

  if (!initiated) {
    interrupt_enable();
    return -1;
  }

  if (lockowner.count(lock) == 0 || lockowner[lock] == NULL || lockowner[lock] != curr) {
    interrupt_enable();
    return -1;
  }

  unlock(lock);
  
  interrupt_enable();
  return 0;
}


int thread_wait(unsigned int lock, unsigned int cond) {

  interrupt_disable();
  if (!initiated || lockowner.count(lock) == 0 || lockowner[lock] == NULL || lockowner[lock] != curr) {
    interrupt_enable();
    return -1;
  }

  unlock(lock);

  pair<unsigned int, unsigned int> key = make_pair(lock,cond);
  if (cvqueue.find(key) == cvqueue.end()){
    queue<TCB*> q1;
    cvqueue[key] = q1;
  }
  cvqueue[key].push(curr);
  swapcontext(curr->ucontext, swi->ucontext);
  interrupt_enable();
  return thread_lock(lock);
}


int thread_signal(unsigned int lock, unsigned int cond){
  interrupt_disable();

  if (!initiated) {
    interrupt_enable();
    return -1;
  }

  pair<unsigned int, unsigned int> key = make_pair(lock,cond);
  if (!cvqueue[key].empty()){
    readyqueue.push(cvqueue[key].front());
    cvqueue[key].pop();
  };
  interrupt_enable();
  return 0;
}

int thread_broadcast(unsigned int lock, unsigned int cond) {
  interrupt_disable();

  if (!initiated) {
    interrupt_enable();
    return -1;
  }

  pair<unsigned int, unsigned int> key = make_pair(lock,cond);
  while (!cvqueue[key].empty()){
    readyqueue.push(cvqueue[key].front());
    cvqueue[key].pop();
  }

  interrupt_enable();
  return 0;
}