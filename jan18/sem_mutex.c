#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include "./threads.h"

// define a new lock type. In this case, we'll
// define it in terms of a semaphore.
typedef sem_t lock_t;

void init_lock(lock_t *l) {
	sem_init(l, 0, 1);
}

void acquire_lock(lock_t *l) {
	sem_wait(l);
	// now we have the lock
}

void release_lock(lock_t *l) {
	sem_post(l);
}

// global lock...
lock_t data_lock;
// ... which locks this data
volatile long data = 0;

long nthreads = 5;
long iters = 100000;


// The function that will run
void *thread_function(void *arg) {
  for (int i = 0; i < iters; i++) {
		acquire_lock(&data_lock);
    data += 1;
		release_lock(&data_lock);
  }
  return NULL;
}

int main(int argc, char **argv) {

	init_lock(&data_lock);
  // we expect `data` to be (nthreads * iters)
  pthread_t threads[nthreads];

  for (int i = 0; i < nthreads; i++) {
    pthread_create(&threads[i], NULL, thread_function, NULL);
  }
  for (int i = 0; i < nthreads; i++) {
    pthread_join(threads[i], NULL);
  }

	if (data != nthreads * iters) {
		fprintf(stderr, "error: data is wrong! (%ld != %ld)\n", data, nthreads * iters);
		exit(EXIT_FAILURE);
	}
	return EXIT_SUCCESS;
}
