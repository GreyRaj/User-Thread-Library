# **ECS 150 Project 2 Report**
## **Team Members: Rajveer Grewal and Ayush Saha**

# **General Summary**

In this project, our team implemented a user-level thread library that enables
users to access to a multi-threading libary. We created functions that handle
preemption, thread creation, thread scheduling, and thrad deletion. Our library
also allows for thread synchronization, using semaphores.

# **Implementation**

## **Phase 1: queue.c**

### **High-Level Implementation**

In *queue.c* we implement a queued data structure that is used for our various
processes to be stored in and manipulated.

The node class defines a struct *node* with two components:*data* points to
data of any type and *next* points to the next node in the queue

The *makenode* function creates a new node with the given data. The *delnode*
function deallocates memory used by the given node and sets the node pointer
to *NULL*. The *queue_find* function looks for the given data in the queue and
returns a pointer to it if found, or *NULL* otherwise.

The queue class defines a struct *queue* with two members: *head* points to the
first node in the queue and *tail* points to the last node in the queue.

The *queue_create* function creates a new empty queue and returns a pointer to
it. The *queue_destroy* function deallocates the memory used by the queue and
all its nodes. The *queue_enqueue* function adds a new element to the back of
the queue. The *queue_dequeue* function removes the first element. The
*queue_delete* function removes the element in the queue with the given data.

### **Design Choices & Testing**

First, we created a node object and allowed the user to enqueue items into the
queue by creating new nodes. We then set up input/output channels using the
*show_q()* function to see the inner workings of the queue. After testing the
queue on an input array, we made sure that the head and tail were set properly.
We then tested the "queue_delete" and "destroy" functions, fixing bugs in how
the head and tail were set. For example, when there was only one item left in
the queue and it was being deleted, we had to set the tail to NULL before
freeing the head in the *destroy()* function to avoid a memory segmentation
fault. To test the *queue_delete()* function, we drew out all possible cases on
paper, structured the code to avoid memory leaks, and tested it on an array.

The *display_q* function displays all the elements of the queue. This was
used for testing and debugging. We also used *queue_test()* in the *apps*
directory to test our functions such as *queue_enqueue()*, *queue_delete*, etc.
	
## **Phase 2: uthread.c**

### **High-Level Implementation**

In *uthread.c* we implemented threads using a queue and manage them by a
scheduler. We declare the following variables and functions: *uthread_tcb*
(which contains fields for thread ID, a pointer to a uthread context, and a
pointer to the top of the thread stack), four thread control block (TCB)
pointers (*tcb_cur*, *tcb_prev*, *tcb_main*, and *tcb_new*) two queue pointers
(*ready_q* and *blocked_q*), and lastly an integer thread count(*thread_cnt*).

The *make_tcb()* function is used to create a new thread. It initializes a new
TCB with a new thread ID, a new stack pointer, and a new context pointer. It
then initializes the context by calling *uthread_ctx_init()*.

The *uthread_current()* function returns a pointer to the current thread's TCB.

The *uthread_yield()* function causes the current thread to yield to the next
thread in the ready queue, meaning its TCB is saved as *tcb_prev*, and the next
TCB is dequeued from the ready queue and saved as *tcb_cur*. The current
thread's context is then swapped with the new thread's context using
*swapcontext()*. The *uthread_yield_special()* function is similar to
*uthread_yield()*, but it does not push the current thread's TCB onto the ready
queue. This function is used for blocking and unblocking threads.

The *showq()* and *showq2()* functions are used for debugging and print the
contents of the ready queue and blocked queue, respectively.

The *uthread_exit()* function destroys the current thread's TCB and switches to
the next thread in the ready queue using *uthread_yield_special()*. The
*uthread_exit_tcb_prev()* function is used to destroy the TCB for the previous
thread that was switched out using *uthread_yield()*.

### **Design Choices & Testing**

We first learned how to use the provided functions to initialize the stack top
and thread context using *uthread_ctx_init()*. We studied the starter code to
understand how to use *swapcontext()*. Our testing of the creation of a queue
of TCBs involved drawing pseudocode on paper and testing it, which led to the
realization that print statements could be used to see thread IDs in the queue.
We generated simple test cases using a simplified version of *uthread_yield()*
to ensure that threads were being properly added to the queue. The process of
*uthread_yield()* was fully understood, and the expected contents of the ready
queue were drawn out on paper at each point in the program.

The *show_q* function was used to display the ready queue at each point to
ensure that it matched expectations. Bugs were found and resolved, such as
restructuring *uthread_exit()* so that the thread would switch context to the
correct next queue, and having a separate test case for the only remaining
thread, the initial (main) thread. We also used the *uthread_hello.c* and
*uthread_yield.c* in the *apps* directory to test the output of our code.

## **Phase 3: sem.c**

### **High-Level Implementation**

The *sem.c* file implements sempahores which are used to synchronize multiple
threads or processes that share common resources. We declared a global variable
*sem_cnt* to keep track of the number of semaphores created. The *semaphore*
struct is defined to hold the *semaphore count*, *blocked queue*, and
*semaphore number*.

The *sem_create* function is used to create a new semaphore. It allocates
memory for the semaphore and initializes its *count*, *queue*, and *number*.

The *sem_destroy* function is used to destroy a semaphore. It checks if the
semaphore is valid and its queue is empty before freeing the memory allocated
for the semaphore.

The *sem_down* function is used to decrement the semaphore count and block the
calling thread if the count is zero. It first checks if the semaphore is valid.
If the semaphore count is greater than zero, it decrements the count and
returns *0*. Otherwise, it adds the calling thread to the semaphore's blocked
queue using the *queue_enqueue* function and yields to the next ready thread
using the *uthread_yield_special* function. The *preempt_disable* function is
called to disable preemption before modifying the semaphore or queue.

The *sem_up* function is used to increment the semaphore count and unblock any
waiting threads. It checks to see if the semaphore is valid. It increments the
semaphore count and extracts as many threads as possible from the blocked queue
using the *deq_into_ready_q* function. This function transfers the head of the
blocked queue into the ready queue of threads and decrements the semaphore
count. The while-loop continues until either the semaphore count is zero or the
blocked queue is empty. The *preempt_enable* function is called to enable
preemption before returning from the function.

It's important to note that we use preemptive scheduling to manage the threads
and ensures that modifications to the semaphore and queue are atomic by
disabling preemption before the operations. This is utlization of preemption is
implemented in Phase 4.

### **Design Choices & Testing**

We drew diagrams to see the process of what happens when a thread does a
*sem_up* and *sem_down*. The diagrams helped to identify the appropriate data
structures to use, such as a blocked queue for threads waiting on a semaphore
when the semaphore count is zero. Each semaphore object points to a blocked
queue of threads waiting on it. During testing, the test cases for *sem_count*
and *sem_simple* were used, and the ready queue was drawn out at each point in
the program to see which thread should be executing next and when yields should
occur. We made a new function to display any semaphore's blocked queue, which
was used as a "breakpoint" for the program. This function helped to ensure that
each semaphore's blocked queue was correct at each point in the program.

## **Phase 4: preempt.c**

### **High-Level Implementation**

In *preempt.c*  we implemented preemptive scheduling mechanism using a timer
interrupt. The *HZ* macro defines the frequency of preemption. The signal
**set* manages signals for enabling/disabling preemption.

The *preemption()* function is the signal handler for the timer interrupt. It
blocks the *SIGALRM* signal, and if the scheduler is ready, it calls
*uthread_yield()* to yield the CPU to the next ready thread. It then unblocks
the *SIGALRM* signal.

The *preempt_disable()* function blocks the *SIGALRM* signal, disabling
preemption. The *preempt_enable()* function unblocks the *SIGALRM* signal,
enabling preemption. 

The *set_up_preempt_handler()* function sets up the signal handler for timer
interrupts. The *set_up_timer()* function sets up the timer to generate a timer
interrupt every 0.01 second, using *setitimer()*. The *set_up_preempt_ena()*
function sets up the signal sets for enabling and disabling preemption.

The *preempt_start()* function initializes all the necessary functions for
preemptive scheduling and enables interrupts by unblocking the *enable_preempt*
signal set. It sets the *ready* flag to *true*. The *preempt_stop()* function
blocks the *disable_preempt* signal set and sets the *ready* flag to *false*.

### **Design Choices & Testing**

During planning, we figured out where to put critical sections, mainly over any
global variable access, and made use of local variables to ensure that we would
still switch context to the new thread even if the execution flow was disrupted
by a timer interrupt. In non-thread-switching areas, we found it useful to put
*disable()* and *enable()* over the entire section to prevent interruption
while modifying or accessing globals. For testing, we made test cases with
different values of timer frequency and printed "INTERRUPT" every time there
was a timer interrupt. We made sure we were switching to the correct next ready
thread, and the ready queue was being properly modified by enqueuing the
current thread of execution and dequeuing the next thread of execution.

# **Resources**

We used lecture slides as well as GNU Manual pages to help guide us.