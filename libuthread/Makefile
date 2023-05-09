
# create tcb struct
# 3 data abt each thread: uthread state, uthread_ctx_t,
# void* stacktop_ptr
# every thread: create TCB, 

# using queue
# ready queue, zombie queue contain ready/zombie threads
# elect_next_uthread()		 use queue dequeue
# uthread ctx switch: send cur. uctx and next uthread ptr

# uthread_create: create uthread TCB ptr, malloc or calloc to dynamically allocate struct to TCB
# struct has the state, ctx switch info, ptr to stack head
# uthread's top of stack = uthread_ctx_alloc_stack()
	# if not sucesffully created, must free top of stack and the dyn. alloc. TCB

# uthread_ctx_init() ~~~> uctx of the TCB, stacktp, func, arg
# if fails, free the TCB's top ptr and free the TCB
# else change state to ready
# if ready, add it to queue using queue_enqueu()


# Target library

$(lib): libuthread.a

headers := private.h uthread.h queue.h
# main: main.o libuthread.a
	# gcc -g -Wall -Wextra -Werror -o $@ $^


####################################
# main: main.o libuthread.a
	# gcc -lm -o main main.o -L. libuthread.a

libuthread.a: uthread.o context.o queue.o preempt.o sem.o
	ar rcs libuthread.a $@ $^
	
# main.o: main.c private.h uthread.h queue.h sem.h
	# gcc -g -Wall -Wextra -Werror -c $@ $^

uthread.o: uthread.c uthread.h private.h queue.h
	gcc -g -Wall -Wextra -Werror -c $@ $^
	
context.o: context.c private.h uthread.h
	gcc -g -Wall -Wextra -Werror -c $@ $^

preempt.o: preempt.c private.h uthread.h
	gcc -g -Wall -Wextra -Werror -c $@ $^

sem.o: sem.c sem.h
	gcc -g -Wall -Wextra -Werror -c $@ $^

queue.o: queue.c queue.h 
	gcc -g -Wall -Wextra -Werror -c $@ $^

# static lib out of object files
all: $(main)

# libuthread.a: libuthread.a queue.o
	# ar rcs libuthread.a queue.o

# uthread.o: uthread.c private.h uthread.h queue.h
	# gcc -g -Wall -Wextra -Werror -c uthread.c


clean:
	rm -fr *.o *.out *.a *.gch *.stackdump *.exe
	