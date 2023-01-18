#pragma once

#include <pthread.h>
#include <unistd.h>

// abstract thread pool management

static inline void create_threads(size_t count, pthread_t *pool, void *(*func)(void*)) {
  for (int i = 0; i < count; i++) {
    pthread_create(&pool[i], NULL, func, NULL);
  }
}

static inline void join_threads(size_t count, pthread_t *pool) {
  for (int i = 0; i < count; i++) {
    pthread_join(pool[i], NULL);
  }
}

// just fib
static int do_work(int input) {
	if (input < 2) return input;
	return do_work(input - 2) + do_work(input - 1);
}
