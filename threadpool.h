#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include <stdio.h>
#include <stdbool.h>

typedef struct tpool tpool_t;

// creates a new threadpool with num threads concurrently active
tpool_t *tpool_create(size_t num);

// adds new work to the queue, which then gets processed by the threads
bool tpool_add_work(tpool_t *tm, void (*func)(void *), void *arg);

// waits for all the threads to finish their work
void tpool_wait(tpool_t *tm);

// destroyes the threadpool and frees all the allocated memory
void tpool_destroy(tpool_t *tm);

#endif