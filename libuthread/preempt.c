#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include <time.h>
#include <unistd.h>

#include "private.h"
#include "uthread.h"

/*
 * Frequency of preemption
 * 100Hz is 100 times per second
 */
#define HZ 100
sigset_t disable_preempt;
sigset_t enable_preempt;
bool ready = false;

void preemption() {
    sigprocmask(SIG_BLOCK, &disable_preempt, NULL);

    //fprintf(stderr, "INTERUPT--\n");
    if (ready)
        uthread_yield();

    sigprocmask(SIG_UNBLOCK, &enable_preempt, NULL);
}




void preempt_disable(void)
{
	sigprocmask(SIG_BLOCK, &disable_preempt, NULL); //The signal SIGVTALRM should be blocked
}

void preempt_enable(void)
{
    if (ready)
	    sigprocmask(SIG_UNBLOCK, &enable_preempt, NULL); //The signal SIGVTALRM should be enabled
}

// Set up signal handler for timer interrupt
void set_up_preempt_handler() {
    struct sigaction sa;
    sa.sa_handler = preemption;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGALRM, &sa, NULL);
}
// Set up timer to generate timer interrupt every 0.1 second
void set_up_timer() {
    struct itimerval timer;
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 1000000/HZ;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 1000000/HZ;
    setitimer(ITIMER_REAL, &timer, NULL);
}
// set up preemption enabling/disabling
void set_up_preempt_ena() {
    /*Disable preempt for accessing critical areas of data structure*/
    sigemptyset(&disable_preempt);
    sigaddset(&disable_preempt, SIGALRM);
    /*After properly managing data structures allow preempt again*/
    sigemptyset(&enable_preempt);
    sigaddset(&enable_preempt, SIGALRM);
}

void preempt_start(bool preempt)
{
    if (!preempt) return;

    // initialize all necessary functions
    set_up_preempt_handler();
    set_up_timer();
    set_up_preempt_ena();

    // enable intrupts
    sigprocmask(SIG_UNBLOCK, &enable_preempt, NULL);

    ready = true;
    //printf("MADE\n");
}

void preempt_stop(void)
{
	sigprocmask(SIG_BLOCK, &disable_preempt, NULL); //The signal SIGVTALRM should be blocked
    ready = false;
}

