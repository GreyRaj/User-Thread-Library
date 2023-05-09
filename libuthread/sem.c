#include <stddef.h>
#include <stdlib.h>

#include "sem.h"
#include "queue.h"
#include "private.h"
#include<ucontext.h>

int sem_cnt = 0;

struct semaphore {
	size_t cnt;
	queue_t blocked_queue;
	int semnum;
};

// create sem
sem_t sem_create(size_t count)
{
	// create new semaphore obj
	sem_t newsem = (sem_t)malloc(sizeof(struct semaphore));
	newsem->cnt = count;
	newsem->semnum = ++sem_cnt;
	newsem->blocked_queue = queue_create();

	return newsem;
}

// destroy sem
int sem_destroy(sem_t sem)
{
	// if semaphore is invalid, or there exist threads waiting on sem.
	if (sem == NULL || sem->blocked_queue->head != NULL) {
		return -1;
	}
	// destroy sem.
	free(sem);
	sem = NULL;
	return 0;
}


// take sem
int sem_down(sem_t sem)
{
	if (sem == NULL) return -1;
	preempt_disable();

	// decrement sem->cnt
	if (sem->cnt > 0) {
		sem->cnt--;
	}
	else {
		// add requester to semaphore's blocked queue
		queue_enqueue(sem->blocked_queue, uthread_current());

		// yield to next ready thread
		uthread_yield_special();		// this will enable preempt once finished
		
	}
	return 0;

}
// give up sem
int sem_up(sem_t sem)
{
	if (sem == NULL) return -1;

	preempt_disable();

	// increment sem->cnt
	sem->cnt++;

	// extract as many threads that are waiting on sem into rq as possible
	while (1) {
		// stop extracting threads into ready queue if no items left
		if (!(sem->cnt > 0 || queue_length(sem->blocked_queue) > 0)) {
			break;
		}
		// transfer sem->blocked_queue head into ready queue
		deq_into_ready_q(sem->blocked_queue);

		// decrement semaphore cnt
		sem->cnt--;
	}
	preempt_enable();
	return 0;
}

