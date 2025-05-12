#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"

/*
 * Frequency of preemption
 * 100Hz is 100 times per second
 */
#define HZ 100
static bool preempt_on;
static sigset_t vt_set;
static struct sigaction old_sa;
static struct itimerval old_timer;

void preempt_disable(void) {
	sigprocmask(SIG_BLOCK, &vt_set, NULL);
}

void preempt_enable(void) {
	sigprocmask(SIG_UNBLOCK, &vt_set, NULL);
}

//not defined
static void preempt_handler(int signum) {
	(void)signum;
	uthread_yield();
}

void preempt_start(bool preempt) {
	preempt_on = preempt;
	if (!preempt_on) return;
	//
	sigemptyset(&vt_set);
	sigaddset(&vt_set, SIGVTALRM);
	//set the handler
	struct sigaction sa = {0};
	sa.sa_handler = preempt_handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGVTALRM, &sa, &old_sa);
	//timer
	struct itimerval timer = {0};
	timer.it_interval.tv_usec = 1000000/HZ; //10k micoseonds
	timer.it_value = timer.it_interval;
	setitimer(ITIMER_VIRTUAL, &timer, &old_timer);
}

void preempt_stop(void) {
	if (!preempt_on) return;
	//reset timer
	setitimer(ITIMER_VIRTUAL, &old_timer, NULL);
	//reset action
	sigaction(SIGVTALRM, &old_sa, NULL);
}