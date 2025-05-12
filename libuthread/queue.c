#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

typedef struct node {
	void *data;
	struct node* next;
} Node;

struct queue {
	Node *head;
	Node *tail;
	size_t count;
};

queue_t queue_create(void) {
	queue_t queue = malloc(sizeof(struct queue));
	if (!queue) return NULL;
	//
	queue->tail = NULL;
	queue->head = NULL;
	queue->count = 0;
	return queue;
}

int queue_destroy(queue_t queue) {
	if (queue == NULL || queue->head != NULL) return -1;
	//
	free(queue);
	return 0;
}

int queue_enqueue(queue_t queue, void *data) {
	if (queue == NULL || data == NULL) {
		return -1;
	}
	//create new node
	Node *new = malloc(sizeof(Node));
	if (!new) return -1; //alloc failed
	new->data = data;
	new->next = NULL;
	//put it into queue
	if (queue->tail == NULL) { //empty
		queue->head = new;
		queue->tail = new; 
	} else { //non-empty
		queue->tail->next = new;
		queue->tail = new;
	}
	queue->count++;
	return 0;
}

int queue_dequeue(queue_t queue, void **data) {
	if (queue == NULL || data == NULL || queue->head == NULL) {
		return -1;
	}
	//fetch data
	Node *cur = queue->head;
	*data = cur->data;
	queue->head = cur->next;
	//dequeue
	if (queue->head == NULL) {
		queue->tail = NULL;
	}
	free(cur);
	queue->count--;
	return 0;
}

int queue_delete(queue_t queue, void *data) {
	if (queue == NULL || data == NULL) {
		return -1;
	}
	//
	Node *cur = queue->head;
	Node *prev = NULL;
	while(cur) {
		if (cur->data == data) {
			//reconnect
			if (prev == NULL) {
				queue->head = cur->next;
			} else {
				prev->next = cur->next;
			}
			if (cur == queue->tail) {
				queue->tail = prev;
			}
			free(cur);
			queue->count--;
			return 0;
		}
		prev = cur;
		cur = cur->next;
	}
	return -1;
}

int queue_iterate(queue_t queue, queue_func_t func) {
	if (queue == NULL || func == NULL) {
		return -1;
	}
	//
	Node *cur = queue->head;
	while (cur) {
		Node *next = cur->next;
		func(queue, cur->data);
		cur = next;
	}
	return 0;
}

int queue_length(queue_t queue) {
	if (queue == NULL) {
		return -1;
	}
	return (int)queue->count;
}

