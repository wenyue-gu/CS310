#include "thread.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <cmath>
using namespace std;

// Enumerate function signatures
int main(int argc, char *argv[]);
void start(void *args);

typedef struct order{
  int cashier;
  int sandwich;
  order* next;
}order;

typedef struct file_thread{
  char* filename;
  int cashier;
}file_thread;

int cashier;
int max_order;
int num_order;
order* board;



bool isbusy(int i){
    order* cur = board;
    while(cur!=NULL){
        if(cur->cashier==i){
            return true;
        }
        cur = cur->next;
    }
    return false;
}

void post(void* th){
    file_thread* in = (file_thread*) th;
    //char* filename = in->filename;
    int c = in->cashier;

    thread_lock(1);
    // ifstream inFile;
    // inFile.open(filename);

    int x;
    while(x<4){
        while(num_order==max_order || num_order==cashier||isbusy(c)){
            thread_signal(1,2);
            thread_wait(1,1);
        }
        order* add = new order;
        add->cashier = c;
        add->sandwich = rand()%80+2;
        add->next = NULL;
        num_order+=1;
        x+=1;
        if(board==NULL){
            board = add;
        }
        else{
            add->next = board->next;
            board->next = add;
        }
        cout << "POSTED: cashier " << c << " sandwich " << x <<endl;
        if(num_order==max_order || num_order==cashier){
            thread_signal(1, 2);
        }
        thread_wait(1,1);
    }

    while(isbusy(c)){
        thread_signal(1,2);
        thread_wait(1,1);
    }
    cashier--;
    if(num_order==max_order || num_order==cashier){
        thread_signal(1,2);
    }
    thread_unlock(1);
    delete(in);
    return;
}


order* closestSandwich(int sand_num){
    order* head = board;
    order* closest;
    order* temp = new order;

    int curdiff = 1000;

    while(head!=NULL){
        int diff = abs(head->sandwich - sand_num);
        if(diff <= curdiff){
            curdiff= diff;
            closest = head;
        }
        head = head->next;
    }

    temp->cashier = closest->cashier;
    temp->sandwich = closest->sandwich;
    temp->next = NULL;

    head = board;
    order* todelete;
    if(head->next){
        while(head->next->next){
            if(head==closest){
                head->cashier = head->next->cashier;
                head->sandwich = head->next->sandwich;
                todelete = head->next;
                head->next = head->next->next;
                delete(todelete);
                return temp;
                }
            head = head->next;
        }
    }
    else{
        todelete = head;
        board = NULL;
        delete(todelete);
        return temp;
    }
    
    if(head==closest){
        head->sandwich = head->next->sandwich;
        head->cashier = head->next->cashier;
    }
    todelete = head->next;
    head->next = NULL;
    delete(todelete);
    return temp;

}

void make(){
    thread_lock(1);
    int n = -1;
    while(cashier>0){
        if(num_order==max_order||num_order==cashier){
            order* tomake = closestSandwich(n);
            int cash = tomake->cashier;
            n = tomake->sandwich;
            delete(tomake);
            cout << "READY: cashier " << cash << " sandwich " << n <<endl;
            num_order-=1;
        }
        thread_broadcast(1,1);
        thread_wait(1,2);
    }
    thread_unlock(1);
    delete(board);
    return;
}

void start(void *args) {
    char** arg = (char**) args;
    max_order = 4;
    void * a;
    for(int i = 0; i<cashier; i++){
        file_thread* thread = new file_thread;
        //thread->filename = arg[i+2];
        thread->cashier = i;
        thread_create((thread_startfunc_t)post, (void*) thread);
    }
    thread_create((thread_startfunc_t)make, a);
    start_preemptions(0, 1, 123);
    return;
}


int main(int argc, char *argv[]) {

    // if (argc < 2) {
    //     cout << "Not enough inputs" << endl;
    //     return (0);
    // }
    cashier = 5;
    thread_libinit((thread_startfunc_t)start, argv);
}