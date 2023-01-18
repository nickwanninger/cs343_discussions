#pragma once

#include "atomic.h" // from pclab
#include <stdlib.h>
#include <pthread.h>

typedef struct {
	int *data;
	int head;
	int len;
	pthread_mutex_t lock;
} stack_t;

static inline void stack_init(stack_t *st, int length) {
	st->data = (int*)calloc(length, sizeof(int));
	pthread_mutex_init(&st->lock, NULL);
	st->len = length;
	st->head = 0;
}

static inline void stack_free(stack_t *st) {
	free(st->data);
}

static inline void stack_push(stack_t *st, int val) {
	while (1) {
		pthread_mutex_lock(&st->lock);
		if (st->head < st->len) {
			break;
		}
		pthread_mutex_unlock(&st->lock); // let someone else pop from the stack
	}
	st->data[st->head++] = val;
	pthread_mutex_unlock(&st->lock);
}

// try to pop a value. If there is nothing return 0. Return 1 otherwise
static inline int stack_pop(stack_t *st, int *out) {
	pthread_mutex_lock(&st->lock);
	if (st->head == 0) {
		pthread_mutex_unlock(&st->lock);
		// fail
		return 0;
	}
	*out = st->data[--st->head];
	pthread_mutex_unlock(&st->lock);

	// success
	return 1;
}
