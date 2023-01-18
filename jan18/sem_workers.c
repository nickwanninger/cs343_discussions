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
#define WORK_COUNT 5000
// the number of worker threads
#define NTHREADS 8
// a magic work value indicating we are done with work
#define WORK_DONE -1

// the stack of work to be done
stack_t work_todo;
// semaphore allowing workers to wait on work
sem_t work_sem;

void post_work(int work) {
  printf("posting work %d\n", work);
  stack_push(&work_todo, work);

  // notify the workers of the work
  sem_post(&work_sem);
}

// get the next bit of work off the stack, blocking until there is work
// available.
int get_work(void) {
  // wait for work to be availabe
  sem_wait(&work_sem);
  int work;
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
	printf("start thread %d\n", my_tid);

  while (1) {
    int work = get_work();
    if (work == WORK_DONE)
      break;
    do_work(work);
    printf("thread %d finished %d\n", my_tid, work);
  }
  printf("thread %d done!\n", my_tid);
  return NULL;
}

int main(int argc, char **argv) {

  // initialize the work stack
  stack_init(&work_todo, 512);

  // we expect `data` to be (nthreads * iters)
  pthread_t threads[NTHREADS];
  create_threads(NTHREADS, threads, thread_function);

  // fire off a bunch of work
  for (int i = 0; i < WORK_COUNT; i++) {
    post_work(rand() % 38); // post random work to be done
  }

  // tell all the threads we are done by posting on the semaphore without
  // pushing to the work_todo stack
  for (int i = 0; i < NTHREADS; i++)
    sem_post(&work_sem);

  join_threads(NTHREADS, threads);

  // free the stack
  stack_free(&work_todo);
  return EXIT_SUCCESS;
}
