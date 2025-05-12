#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"
#include "queue.h"

typedef enum states {
	READY,
	RUNNING,
	BLOCKED,
	TERMINATED
} States;

typedef struct uthread_tcb {
	States state;
	uint32_t tid;
	//
	void *stack;
	ucontext_t ctx;
} Uthread_tcb;

typedef struct {
	queue_t ready_q;
	queue_t blocked_q;
	//
	uint32_t next_tid;
	Uthread_tcb idle;
	Uthread_tcb *current;
} uthread_running;

static uthread_running ur;

struct uthread_tcb *uthread_current(void) {
	/* TODO Phase 2/3 */
	return ur.current;
	// if (ur.current->state == READY) {
	// }
	// return NULL;
}

void uthread_yield(void) {
	// ur.current = uthread_current();
	if (!ur.current) return;
	ur.current->stack = READY;
	queue_enqueue(ur.ready_q, ur.current);
	uthread_ctx_switch(&ur.current->ctx, 
					   &ur.idle.ctx);
}

void uthread_exit(void) {
	Uthread_tcb *cur = uthread_current();
	cur->state = TERMINATED;
	uthread_ctx_switch(&cur->ctx, &ur.idle.ctx);
}

void failedToCreate(Uthread_tcb *thread_tcb) {
	uthread_ctx_destroy_stack(thread_tcb->stack);
	free(thread_tcb);
}

int uthread_create(uthread_func_t func, void *arg) {
	//set up tcb
	Uthread_tcb *thread_tcb = malloc(sizeof(Uthread_tcb));
	if (!thread_tcb) return -1;
	//allocate stack
	thread_tcb->stack = uthread_ctx_alloc_stack();
	if (!thread_tcb->stack) {
		failedToCreate(thread_tcb);
		return -1;
	}
	//create thread ctx
	if (uthread_ctx_init(&thread_tcb->ctx, 
						 thread_tcb->stack, func, arg) == -1) {
		failedToCreate(thread_tcb);
		return -1;	
	}
	//set up tcb
	thread_tcb->state = READY;
	thread_tcb->tid = ur.next_tid++;
	//push it into the queue
	if (queue_enqueue(ur.ready_q, thread_tcb) == -1) {
		failedToCreate(thread_tcb);
		return -1;
	}
	return 0;
}

void schedule_loop(void *arg) {
	(void)arg;
	while (1) {
		if (queue_length(ur.ready_q) > 0) {
			//gets the top queued thread
			queue_dequeue(ur.ready_q, (void**)&ur.current);
			if (!ur.current) continue;
			ur.current->state = RUNNING;
			//switches to thread
			uthread_ctx_switch(&ur.idle.ctx, 
							   &ur.current->ctx);
		} else if (queue_length(ur.blocked_q) > 0){
			//wait for I/0 or timer 
		} else {
			break;
		}
	}
}

int uthread_run(bool preempt, uthread_func_t func, void *arg) {
	//set up
	ur.ready_q = queue_create(); 
	ur.blocked_q = queue_create();
	ur.next_tid = 0;
	//create scheuler stack
	ur.idle.stack = uthread_ctx_alloc_stack();
	if (!ur.idle.stack) {
		uthread_ctx_destroy_stack(ur.idle.stack);
		return -1;
	}
	//set up scheuler ctx
	if (uthread_ctx_init(&ur.idle.ctx, ur.idle.stack,
						 schedule_loop, NULL) == -1) {
		uthread_ctx_destroy_stack(ur.idle.stack);
		return -1;
	}
	//set up tcb
	ur.idle.state = RUNNING;
	ur.idle.tid = 0;
	//create inital thread
	if (uthread_create(func, arg) == -1) return -1;
	
	//preemptive
	preempt_start(preempt);
	
	//scheuling
	schedule_loop(NULL);
	//all threads completed | free
	queue_destroy(ur.ready_q);
	queue_destroy(ur.blocked_q);
	uthread_ctx_destroy_stack(ur.idle.stack);
	return 0;
}

void uthread_block(void) {
	/* TODO Phase 3 */
	return;
}

void uthread_unblock(struct uthread_tcb *uthread) {
	/* TODO Phase 3 */
	uthread->tid = 0;
	return;
}