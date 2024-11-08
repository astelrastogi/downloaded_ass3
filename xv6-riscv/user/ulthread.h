#ifndef __UTHREAD_H__
#define __UTHREAD_H__

#include <stdbool.h>

#define MAXULTHREADS 100

enum ulthread_state {
  FREE,
  RUNNABLE,
  RUNNING,
  YIELD,
};

enum ulthread_scheduling_algorithm {
  ROUNDROBIN,   
  PRIORITY,     
  FCFS,         // first-come-first serve
};

struct context {
  uint64 ra;
  uint64 sp;

  uint64 s0;
  uint64 s1;
  uint64 s2;
  uint64 s3;
  uint64 s4;
  uint64 s5;
  uint64 s6;
  uint64 s7;
  uint64 s8;
  uint64 s9;
  uint64 s10;
  uint64 s11;

  uint64 a0;
  uint64 a1;
  uint64 a2;
  uint64 a3;
  uint64 a4;
  uint64 a5;
};

typedef struct ulthread {
  uint64 tid;                      // Thread ID
  uint64 stptr;                    // Pointer to the stack memory
  struct context context_saved_at; // Context save location
  enum ulthread_state state;       // Thread state
  int priority;                    // Priority level
  uint64 args[100];                // Array of arguments
  uint64 start;                    // Starting point of each thread
} ulthread_t;

extern ulthread_t ulthreads[MAXULTHREADS];  // Array to hold all threads
extern int current_tid;                     // Tracks current running thread
extern int scheduling_algorithm;            // Chosen scheduling algorithm

void ulthread_init(int schedalgo);
bool ulthread_create(uint64 start, uint64 stack, uint64 args[], int priority);
void ulthread_schedule(void);
void ulthread_yield(void);
void ulthread_destroy(void);
int get_current_tid(void);

#endif