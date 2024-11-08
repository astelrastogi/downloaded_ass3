/* CSE 536: User-Level Threading Library */
#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "user/ulthread.h"

/* Standard definitions */
#include <stdbool.h>
#include <stddef.h> 

ulthread_t ulthreads[MAXULTHREADS];  // Array to hold all threads
int current_tid = -1;  // No thread running initially
int scheduling_algorithm = ROUNDROBIN; //Default algorithm assumption

/* Get thread ID */
int get_current_tid(void) {
    return current_tid;
}

/* Thread initialization */
void ulthread_init(int schedalgo) {
    for (int i = 1; i < MAXULTHREADS; i++) {
        ulthreads[i].tid = i;
        ulthreads[i].state = FREE;
        ulthreads[i].priority = -1;
        ulthreads[i].start = 0;
        ulthreads[i].stptr = 0;
    }
    current_tid = 0;
    ulthreads[0].state = RUNNING;
    scheduling_algorithm = schedalgo;
}

/* Thread creation */
bool ulthread_create(uint64 start, uint64 stack, uint64 args[], int priority) {
    for (int i = 1; i < MAXULTHREADS; i++) {
        if (ulthreads[i].state == FREE) {
            ulthreads[i].state = RUNNABLE;
            ulthreads[i].priority = priority;
            ulthreads[i].start = start;
            ulthreads[i].stptr = stack;
            
            memset(&ulthreads[i].context_saved_at, 0, sizeof(ulthreads[i].context_saved_at));
            
            ulthreads[i].context_saved_at.ra = start;
            ulthreads[i].context_saved_at.sp = stack;
            
            ulthreads[i].context_saved_at.a0 = args[0];
            ulthreads[i].context_saved_at.a1 = args[1];
            ulthreads[i].context_saved_at.a2 = args[2];
            ulthreads[i].context_saved_at.a3 = args[3];
            ulthreads[i].context_saved_at.a4 = args[4];
            ulthreads[i].context_saved_at.a5 = args[5];

            /* Please add thread-id instead of '0' here. */
            printf("[*] ultcreate(tid: %d, ra: %p, sp: %p)\n", i, start, stack);

            return true;
        }
    }
    return false;  // No free slots available
}

/* Thread scheduler */
void make_yield_threads_runnable(void) {
    for (int yield_tid = 1; yield_tid < MAXULTHREADS; yield_tid++) {
        if (ulthreads[yield_tid].state == YIELD) {
            ulthreads[yield_tid].state = RUNNABLE;
            break;
        }
    }
}

int find_next_runnable_round_robin(void) {
    for (int i = (current_tid + 1) % MAXULTHREADS; i != current_tid; i = (i + 1) % MAXULTHREADS) {
        if (ulthreads[i].state == RUNNABLE) {
            return i;
        }
    }
    return -1;
}

int find_next_runnable_priority(void) {
    int next_tid = -1;
    int highest_priority = -1;

    for (int i = (current_tid + 1) % MAXULTHREADS; i != current_tid; i = (i + 1) % MAXULTHREADS) {
        if (ulthreads[i].state == RUNNABLE && ulthreads[i].priority > highest_priority) {
            next_tid = i;
            highest_priority = ulthreads[i].priority;
        }
    }
    return next_tid;
}

int find_next_runnable_fcfs(void) {
    for (int i = 1; i < MAXULTHREADS; i++) {
        if (ulthreads[i].state == RUNNABLE) {
            return i;
        }
    }
    return -1;
}

void ulthread_schedule(void) {
    int next_tid = -1;

    make_yield_threads_runnable();

    switch (scheduling_algorithm) {
        case ROUNDROBIN:
            next_tid = find_next_runnable_round_robin();
            break;
        case PRIORITY:
            next_tid = find_next_runnable_priority();
            break;
        case FCFS:
            next_tid = find_next_runnable_fcfs();
            break;
    }

    if (next_tid != -1) {
        if (current_tid != next_tid) {
            int prev_tid = current_tid;
            current_tid = next_tid;
            printf("[*] ultschedule (next tid: %d)\n", current_tid);
            ulthread_context_switch(&ulthreads[prev_tid].context_saved_at, &ulthreads[next_tid].context_saved_at);
            ulthreads[current_tid].state = RUNNING;
        }
    } else {
        int flag = 0;
        for (int i = 0; i < MAXULTHREADS; i++) {
            if (ulthreads[i].state == RUNNABLE) {
                flag = 1;
                printf("[*] ultschedule (next tid: %d)\n", current_tid);
                break;
            }
        }
        if (flag == 0) {
            ulthread_context_switch(&ulthreads[current_tid].context_saved_at, &ulthreads[0].context_saved_at);
        }
    }
}

/* Yield CPU time to some other thread. */
void ulthread_yield(void) {
    if (current_tid != -1) {
        /* Please add thread-id instead of '0' here. */
        printf("[*] ultyield(tid: %d)\n", current_tid);
        ulthreads[current_tid].state = YIELD;
        ulthread_schedule();  // Call the scheduler to pick the next thread
    } 
}

/* Destroy thread */
void ulthread_destroy(void) {
    if (current_tid != -1) {
        printf("[*] ultdestroy(tid: %d)\n", current_tid);
        ulthreads[current_tid].state = FREE;
        ulthread_schedule();  // Call the scheduler to pick the next thread
    }
}
