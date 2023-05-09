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
#include<ucontext.h>

struct uthread_tcb* tcb_cur = NULL;		// current thread tcb
struct uthread_tcb* tcb_prev = NULL;	// prev. thread tcb
struct uthread_tcb* tcb_main = NULL;	// tcb for init. thread (main)
struct uthread_tcb* tcb_new = NULL;		// tcb for newly created threads
queue_t ready_q = NULL;					// ready queue
queue_t blocked_q = NULL;				// blocked queue
int thread_cnt = 0;						// count of created threads

struct uthread_tcb {
	int tid;
	uthread_ctx_t* ctx;
	void* stacktop;
};

// debugging function: prints "msg: tcb->tid"
void er(void* tcb, char* msg) {
	struct uthread_tcb* tcb_ = (struct uthread_tcb*)tcb;
	fprintf(stderr, "%s: %d\n", msg, tcb_->tid);
}

// set tcb_cur := ready_q.pop()
int deq_ready_q() {
	struct uthread_tcb** utcb_tcb_cur_ptr = &tcb_cur;
	return queue_dequeue(ready_q, (void**)utcb_tcb_cur_ptr);
}

// display all TCBs in ready_q
void show_q(int instance) {
	// critical section
	preempt_disable();

	struct node* itr = ready_q->head;
	int i = 0;
	fprintf(stderr, "SHOWING QUEUE: %d\n", instance);
	while (itr != NULL) {
		struct uthread_tcb* mydata = itr->data;
		fprintf(stderr, "item #%d: %d\n", i++, mydata->tid);
		itr = itr->next;
	}
	preempt_enable();
}

// display all items in any queue
void show_any_q(void* some_q, int instance) {
	// crit. section
	preempt_disable();

	struct node* itr = ((queue_t)some_q)->head;
	int i = 0;
	fprintf(stderr, "SHOWING SEM QUEUE: %d\n", instance);
	while (itr != NULL) {
		struct uthread_tcb* mydata = itr->data;
		fprintf(stderr, "item #%d: %d\n", i++, mydata->tid);
		itr = itr->next;

	}
	preempt_enable();
}

// specialized yield that doesn't push tcb_cur to ready_q
void uthread_yield_special() {

	preempt_disable();

	// save tcb_cur into tcb_prev
	tcb_prev = tcb_cur;

	// tcb_cur := ready_q->head
	deq_ready_q();

	// save tcb_cur & prev as locals before enabling preemption
	struct uthread_tcb* tcb_cur_saved = tcb_cur;
	struct uthread_tcb* tcb_prev_saved = tcb_prev;
	preempt_enable();

	if (tcb_prev_saved == tcb_cur_saved) return;

	// jump to execute new tcb
	swapcontext(tcb_prev_saved->ctx, tcb_cur_saved->ctx);
}

// create TCB
void* make_tcb(uthread_func_t func, void* arg) {
	// init. TID, stacktop, ctx
	struct uthread_tcb* tcb_new_ = (struct uthread_tcb*)malloc(sizeof(struct uthread_tcb));
	tcb_new_->tid = thread_cnt++;
	tcb_new_->stacktop = uthread_ctx_alloc_stack();
	tcb_new_->ctx = (uthread_ctx_t*)malloc(sizeof(uthread_ctx_t));
	// initialize ctx
	if (uthread_ctx_init(tcb_new_->ctx, tcb_new_->stacktop, func, arg) != 0) return NULL;
	return tcb_new_;
}


// ready_q.enqueue( some_q.pop() )
void deq_into_ready_q(void* some_q) {
	queue_dequeue((queue_t)some_q, (void**)&tcb_new);
	queue_enqueue(ready_q, tcb_new);
}


// return tcb_cur TCB
struct uthread_tcb* uthread_current(void) {
	return tcb_cur;
}

// yield execution to next item in ready_q
void uthread_yield(void) {

	preempt_disable();
	
	// save tcb_cur into tcb_prev
	tcb_prev = tcb_cur;
	queue_enqueue(ready_q, tcb_cur);

	// tcb_cur := ready_q->head
	deq_ready_q();

	// save cur/prev tcbs as locals just in case
	struct uthread_tcb* tcb_cur_saved = tcb_cur;
	struct uthread_tcb* tcb_prev_saved = tcb_prev;
	preempt_enable();

	// no need to switch to same thd
	if (tcb_prev_saved == tcb_cur_saved) return;
	// execute new thd
	swapcontext(tcb_prev_saved->ctx, tcb_cur_saved->ctx);
}

// destroy tcb_prev TCB
void uthread_exit_tcb_prev(void)
{
	// destroy tcb_cuready_q stack
	uthread_ctx_destroy_stack(tcb_prev->stacktop);
	tcb_prev->stacktop = NULL;
	// set tcb_prev to null
	free(tcb_prev);
	tcb_prev = NULL;
}



// kill tcb_cur and switch to execute ready_q.pop(), or switch back to init thread
void uthread_exit(void) {

	// crit. section (entire function)
	preempt_disable();

	// save tcb_cur->ctx
	uthread_ctx_t saved_tcb_cur_ctx = *tcb_cur->ctx;

	uthread_ctx_destroy_stack(tcb_cur->stacktop);	// destroy tcb_cuready_q stack
	tcb_cur->stacktop = NULL;
	//free(tcb_cur);		// free uthread_ctx_t
	tcb_cur = NULL;


	// switch execution flow to next thread
	if (deq_ready_q() == 0) {
		// switch to executing first item in ready_q 
		struct uthread_tcb* tcb_cur_updated = tcb_cur;
		preempt_enable();
		swapcontext(&saved_tcb_cur_ctx, tcb_cur_updated->ctx);
	}
	else {
		// switch to executing init thread
		struct uthread_tcb* tcb_main_updated = tcb_main;
		preempt_enable();
		swapcontext(&saved_tcb_cur_ctx, tcb_main_updated->ctx);
	}
}

// tcb_new = new tcb
// ready_q.enqueue(tcb_new)
int uthread_create(uthread_func_t func, void* arg)
{
	// crit. section (entire function)
	preempt_disable();
	
	// create tcb_new
	tcb_new = (struct uthread_tcb*)make_tcb(func, arg);
	if (tcb_new == NULL) return -1;
	if (queue_enqueue(ready_q, tcb_new) != 0) return -1;

	preempt_enable();
	return 0;
}




int uthread_run(bool preempt, uthread_func_t func, void* arg) {
	// create queue
	if (ready_q == NULL) ready_q = queue_create();
	if (ready_q == NULL) return -1;

	// init tcb_main thd
	if (tcb_main == NULL) {
		if (uthread_create(func, arg) != 0) return -1;
		tcb_main = tcb_new; tcb_prev = tcb_new;
		if (queue_destroy(ready_q) != 0) return -1;
	}

	// tcb_cur := new tcb
	if (uthread_create(func, arg) != 0) return -1;
	tcb_cur = tcb_new;

	// init. preempts
	preempt_start(preempt);

	while (1) {
		//~~~~~~~~crit. section 1~~~~~~~~
		preempt_disable();
		if (deq_ready_q() != 0) {
			preempt_enable();
			break;
		}
		// save cur/prev tcbs before enabling preemption
		struct uthread_tcb* tcb_prev_saved = tcb_prev;
		struct uthread_tcb* tcb_cur_saved = tcb_cur;
		preempt_enable();

		// execute the newly popped tcb
		//er(tcb_cur_saved, "tcb cur saved");
		swapcontext(tcb_prev_saved->ctx, tcb_cur_saved->ctx);

		//~~~~~~~~crit. section 2~~~~~~~~
		preempt_disable();
		// destroy previous TCB
		uthread_exit_tcb_prev();
		tcb_prev = tcb_cur;
		preempt_enable();
	}
	return 0;
}
// block current tcb
void uthread_block(void)
{
	// crit. section (entire function)
	preempt_disable();

	// put current tcb into blocked_q
	if (blocked_q == NULL) blocked_q = queue_create();
	queue_enqueue(blocked_q, tcb_cur);

	// execute first item in ready_q
	uthread_yield_special();		// yield will auto-reenable preemption (once finished)
	
}
// unblock certain tcb
void uthread_unblock(struct uthread_tcb* uthread)
{
	preempt_disable();

	// unblock uthread, and enqueue uthread into ready_q
	queue_delete(blocked_q, uthread);
	queue_enqueue(ready_q, uthread);

	preempt_enable();
}

