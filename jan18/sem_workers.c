#define _GNU_SOURCE

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "./atomic.h"
#include "./stack.h"
#include "./threads.h"

// the maximum amount of work allowed in the stack
#define WORK_COUNT 50000
// the number of worker threads
#define NTHREADS 8
// a magic work value indicating we are done with work
#define WORK_DONE -1

// the stack of work to be done
stack_t work_todo;
// semaphore allowing workers to wait on work
sem_t work_sem;
long work_done = 0; // how much work has completed

void worker_notify(void) {
  // TODO: using a semaphore, notify the workers of the work in the stack
	sem_post(&work_sem); // increment the semaphore, waking up anyone who was waiting
}

void worker_wait(void) {
  // TODO: using a semaphore, wait for work to be availabe
	sem_wait(&work_sem);
}

void post_work(int work) {
  printf("posting work %d\n", work);
  // push to the stack
  stack_push(&work_todo, work);
  worker_notify();
}

// get the next bit of work off the stack, blocking until there is work
// available.
int get_work(void) {
  int work;
  worker_wait();
  if (stack_pop(&work_todo, &work) == 0) {
    return WORK_DONE;
  }
  return work;
}

// The function that will run
void *thread_function(void *arg) {
  // create a "thread id" (ignore if you want to)
  static int next_tid = 0;
  int my_tid = atomic_fetch_and_add(&next_tid, 1);
  // printf("start thread %d\n", my_tid);

  while (1) {
    int work = get_work();
    if (work == WORK_DONE)
      break;
    do_work(work);
    // printf("thread %d finished %d\n", my_tid, work);
    atomic_fetch_and_add(&work_done, 1); // record that the work completed
  }
  printf("thread %d done!\n", my_tid);
  return NULL;
}

int main(int argc, char **argv) {

  // initialize the work stack
  stack_init(&work_todo, 512);

  // TODO: initialize the semaphore
	sem_init(&work_sem, 0, 0); // 0 means there's no work yet.

  // we expect `data` to be (nthreads * iters)
  pthread_t threads[NTHREADS];
  create_threads(NTHREADS, threads, thread_function); // in threads.h

  // fire off a bunch of work
  for (int i = 0; i < WORK_COUNT; i++) {
    post_work(rand() % 40); // post random work to be done
    usleep(1000); // sleep for a bit before posting the next bit of work
  }

  // tell all the threads we are done by posting on the semaphore without
  // pushing to the work_todo stack
  for (int i = 0; i < NTHREADS; i++) {
    worker_notify();
  }

  // wait on the threads
  join_threads(NTHREADS, threads); // in threads.h
  // free the stack
  stack_free(&work_todo);

  // work_done must be WORK_COUNT
  if (work_done != WORK_COUNT) {
    fprintf(stderr, "ERROR: incorrect amount of work was done: %ld != %d\n",
            work_done, WORK_COUNT);
    exit(EXIT_FAILURE);
  }

  printf("correctly performed %d units of work\n", WORK_COUNT);
  return EXIT_SUCCESS;
}
