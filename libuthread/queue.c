#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include<stdio.h>
#include <stdbool.h>

#include "queue.h"
#include<assert.h>
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// node class

//struct node {
//	void* data;
//	struct node* next;
//};

struct node* makenode(void* data) {
	struct node* newn = (struct node*)malloc(1 * sizeof(struct node));
	newn->next = NULL;
	// printf("%d->", *data_converted);
	newn->data = data;

	return newn;
}

void delnode(struct node** node) {
	free(*node);
	*node = NULL;
}

void* queue_find(queue_t queue, void* data) {
	if (queue == NULL || data == NULL) return NULL;

	struct node* itr = queue->head;
	while (itr != NULL) {
		if (itr->data == data) return itr->data;
		itr = itr->next;
	}
	return NULL;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// queue class

//struct queue {
//	struct node* head;
//	struct node* tail;
//};

// initialize queue
// question: how to check for alloc failure?
queue_t queue_create(void) {
	queue_t newq = (queue_t)malloc(1 * sizeof(struct queue));
	newq->head = NULL;
	newq->tail = NULL;
	return newq;
}

// destroy queue
// question: do we destroy q ONLY IF empty? what to return if non-empty?
int queue_destroy(queue_t queue)
{
	if (queue == NULL) return -1;

	struct node* itr = queue->head;
	queue->head = NULL; queue->tail = NULL;

	while (itr != NULL) {
		// increment itr
		struct node* prev = itr;
		itr = itr->next;

		// delete prev node
		prev->next = NULL;
		//free(prev);
		prev = NULL;
	}
	return 0;
}

// display queue (ASSUMING TYPE INT)
void display_q(queue_t queue) {
	struct node* newn = queue->head;
	
	// printf("%d", newn->next->data);
	if (newn == NULL) {
		printf("EMPTY\n");
		return;
	}
	int* dat_conv;
	while (newn != NULL) {
		dat_conv = newn->data;
		printf("%d ", *dat_conv);
		newn = newn->next;
	} printf("\n");
	return;
}

// add elem to queue
// question: ehcck method: checking for mem alloc failure
int queue_enqueue(queue_t queue, void* data)
{
	// int* data_converted = (int*) (data);
	// printf("%d->", *data_converted);
	if (queue == NULL || data == NULL) return -1;

	struct node* newn = makenode(data);
	if (queue->tail == NULL) {
		//if (queue->head != NULL)
		//{
		//	printf("Error: queue->head is not null while tail is null");
		//}
		queue->tail = newn;
		queue->head = newn;
	}
	else {
		queue->tail->next = newn;
		queue->tail = newn;
	}
	return newn ? 0 : -1;		// return -1 in case mem alloc failure
}


int queue_dequeue(queue_t queue, void** data)
{
	if (queue == NULL || data == NULL || queue->head == NULL) return -1;

	// increment head & (maybe) tail
	struct node* itr = queue->head;
	*data = queue->head->data;		// pop from front of queue
	queue->head = queue->head->next;

	if (queue->head == NULL) queue->tail = NULL;
	delnode(&itr);

	return 0;
}


int queue_delete(queue_t queue, void* data)
{
	// case 1: head == NULL
	if (queue == NULL || data == NULL || queue->head == NULL) return -1;

	// case 2: head->next == NULL
	bool founditem = 0;
	if (queue->head->next == NULL && queue->head->data == data) {
		queue->tail = NULL;
		delnode(&queue->head); // delnode(&queue->tail);
		founditem = 1;
		return 0;
	}

	// cases 3+: node is in middle
	struct node* itr = queue->head;
	struct node* prev = NULL;

	while (itr != NULL) {
		if (itr->data == data) {
			founditem = 1;

			// case 3: at head
			if (itr == queue->head) {
				struct node* todelete = itr;
				queue->head = queue->head->next;
				itr = queue->head;
				delnode(&todelete);
			}
			// case 4: at tail
			else if (itr == queue->tail) {
				queue->tail = prev;
				queue->tail->next = NULL;
				// printf("+%d+", queue->tail->data);
				struct node* todelete = itr;
				delnode(&todelete);
				// printf("->%d<-", queue->tail->next->data);
				return 0;
			}
			// case 5: at middle
			else if (prev != NULL) {
				prev->next = itr->next;
				itr->next = NULL; // free(itr);
				itr = prev->next;
				continue;
			}
		}
		prev = itr;
		itr = itr->next;
	}
	if (founditem) return 0;
	return -1;

	// return (int) (queue == NULL && data == NULL);
}

int queue_iterate(queue_t queue, queue_func_t func)
{
	if (queue == NULL || func == NULL) return -1;

	struct node* itr = queue->head;
	while (itr != NULL) {
		func(queue, itr);
		itr = itr->next;
	}
	return 0;
}

// queue length
int queue_length(queue_t queue)
{
	if (queue == NULL) return -1;

	struct node* itr = queue->head;
	int count = 0;
	while (itr != NULL) {
		count++; itr = itr->next;
	}
	return count;
	// return (int) (queue == NULL);
}



