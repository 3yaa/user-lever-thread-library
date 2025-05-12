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
	BLOCKED, //lwk not used
	TERMINATED
} States;

typedef struct uthread_tcb {
	States state;
	uint32_t tid;
	//
	void *stack;
	ucontext_t ctx;
} uthread_tcb;

typedef struct {
	queue_t ready_q;
	//
	uint32_t next_tid;
	uthread_tcb idle;
	uthread_tcb *current;
} uthread_running;

static uthread_running ur;

void freeTcb(uthread_tcb *thread_tcb) {
	uthread_ctx_destroy_stack(thread_tcb->stack);
	free(thread_tcb);
}

struct uthread_tcb *uthread_current(void) {
	return ur.current;
}

void uthread_yield(void) {
	if (!ur.current) return;
	//
	preempt_disable();
	if (ur.current->state == RUNNING) {
		ur.current->stack = READY;
		queue_enqueue(ur.ready_q, ur.current);
	}
	preempt_enable();
	//
	uthread_ctx_switch(&ur.current->ctx, 
					   &ur.idle.ctx);
}

//only disables preempt since yield re-enables it
void uthread_exit(void) {
	preempt_disable();
	ur.current->state = TERMINATED;
	uthread_yield();
}

int uthread_create(uthread_func_t func, void *arg) {
	//set up tcb
	uthread_tcb *thread_tcb = malloc(sizeof(uthread_tcb));
	if (!thread_tcb) return -1;
	//allocate stack
	thread_tcb->stack = uthread_ctx_alloc_stack();
	if (!thread_tcb->stack) {
		freeTcb(thread_tcb);
		return -1;
	}
	//create thread ctx
	if (uthread_ctx_init(&thread_tcb->ctx, 
						 thread_tcb->stack, func, arg) == -1) {
		freeTcb(thread_tcb);
		return -1;	
	}
	//
	preempt_disable();
	//set up tcb
	thread_tcb->state = READY;
	thread_tcb->tid = ur.next_tid++;
	//push it into the queue
	if (queue_enqueue(ur.ready_q, thread_tcb) == -1) {
		freeTcb(thread_tcb);
		return -1;
	}
	preempt_enable();
	return 0;
}

void idle_loop(void *arg) {
	(void)arg;
	while (1) {
		preempt_disable();
		if (queue_length(ur.ready_q) > 0) {
			//gets the top queued thread
			queue_dequeue(ur.ready_q, (void**)&ur.current);
			if (!ur.current) continue;
			ur.current->state = RUNNING;
			//switches to thread
			preempt_enable();
			uthread_ctx_switch(&ur.idle.ctx, 
							   &ur.current->ctx);
			preempt_disable();
			if (ur.current->state == TERMINATED)
				freeTcb(ur.current);
			preempt_enable();
		} else {
			break;
		}
	}
}

int uthread_run(bool preempt, uthread_func_t func, void *arg) {
	//set up
	ur.ready_q = queue_create(); 
	ur.next_tid = 0;
	//create scheuler stack
	ur.idle.stack = uthread_ctx_alloc_stack();
	if (!ur.idle.stack) {
		uthread_ctx_destroy_stack(ur.idle.stack);
		return -1;
	}
	//set up scheuler ctx
	if (uthread_ctx_init(&ur.idle.ctx, ur.idle.stack,
						 idle_loop, NULL) == -1) {
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
	idle_loop(NULL);
	//all threads completed | free
	queue_destroy(ur.ready_q);
	uthread_ctx_destroy_stack(ur.idle.stack);
	return 0;
}

void uthread_block(void) {
	preempt_disable();
	ur.current->state = BLOCKED;
	preempt_enable();
	//
	uthread_yield();
	return;
}

void uthread_unblock(struct uthread_tcb *uthread) {
	preempt_disable();

	uthread->state = READY;
	queue_enqueue(ur.ready_q, uthread);

	preempt_enable();
	return;
}