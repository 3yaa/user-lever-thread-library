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
static bool preempt_on = false;

void preempt_disable(void) {
	if (!preempt_on) return;
	preempt_on = false;
}

void preempt_enable(void) {
	if (preempt_on) return;
	preempt_on = true;
}

//not defined
static void preempt_handler(int signum) {
	(void)signum;
	if (!preempt_on) return;
	uthread_yield();
}

void preempt_start(bool preempt) {
	preempt_on = preempt;
	if (!preempt_on) return;
	//set the handler
	struct sigaction sa;
	sa.sa_handler = preempt_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGVTALRM, &sa, NULL);
	//timer
	struct itimerval timer = {0};
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 1000000/HZ; //10k micoseonds
	timer.it_value = timer.it_interval;
	setitimer(ITIMER_VIRTUAL, &timer, NULL);
}

void preempt_stop(void) {
	if (!preempt_on) return;
	//reset timer
	struct itimerval timer = {0};
	setitimer(ITIMER_VIRTUAL, &timer, NULL);
	//reset action
	struct sigaction sa;
	sa.sa_handler = SIG_DFL;
	sigaction(SIGVTALRM, &sa, NULL);
	preempt_on = false;
}

