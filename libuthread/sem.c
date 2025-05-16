#include <stddef.h>
#include <stdlib.h>

#include "queue.h"
#include "private.h"
#include "sem.h"

struct semaphore {
	queue_t blocked_q;
	size_t count;
};

sem_t sem_create(size_t count) {
	sem_t sem = malloc(sizeof(*sem));
	sem->count = count;
	sem->blocked_q = queue_create();
	return sem;
}

int sem_destroy(sem_t sem) {
	if (sem == NULL || queue_length(sem->blocked_q) > 0)
		return -1;
	//
	queue_destroy(sem->blocked_q);
	free(sem);
	return 0;
}

int sem_down(sem_t sem) {
	if (sem == NULL) return -1;
	//
	preempt_disable();
	if (sem->count > 0) {
		sem->count--;
		preempt_enable();
		return 0;
	}
	//
	struct uthread_tcb *cur = uthread_current();
	queue_enqueue(sem->blocked_q, cur);
	preempt_enable();
	uthread_block();
	return 0;
}

int sem_up(sem_t sem) {
	if (sem == NULL) return -1;
	//
	preempt_disable();
	if (queue_length(sem->blocked_q) > 0) {
		struct uthread_tcb *cur;
		queue_dequeue(sem->blocked_q, (void**)&cur);
		uthread_unblock(cur);
	} else {
		sem->count++;
	}
	preempt_enable();
	return 0;
}