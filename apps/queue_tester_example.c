#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <queue.h>

#define TEST_ASSERT(assert)				\
do {									\
	printf("ASSERT: " #assert " ... ");	\
	if (assert) {						\
		printf("PASS\n");				\
	} else	{							\
		printf("FAIL\n");				\
		exit(1);						\
	}									\
} while(0)

/* Create */
void test_create(void)
{
	fprintf(stderr, "*** TEST create ***\n");

	TEST_ASSERT(queue_create() != NULL);
}

/* Enqueue/Dequeue simple */
void test_queue_simple(void)
{
	int data = 3, *ptr;
	queue_t q;

	fprintf(stderr, "*** TEST queue_simple ***\n");

	q = queue_create();
	queue_enqueue(q, &data);
	queue_dequeue(q, (void**)&ptr);
	TEST_ASSERT(ptr == &data);
}

//
void test_create_destroy_empty() {
	fprintf(stderr, "*** TEST c_d_e ***\n");
	queue_t q = queue_create();
	TEST_ASSERT(q != NULL);
	TEST_ASSERT(queue_length(q) == 0);
	TEST_ASSERT(queue_destroy(q) == 0);
}

void test_destroy_non_empty() {
	fprintf(stderr, "*** TEST d_n_e ***\n");
	int data = 1;
	queue_t q = queue_create();
	queue_enqueue(q, &data);
	TEST_ASSERT(queue_destroy(q) == -1); 
	void *out;
	queue_dequeue(q, &out);
	TEST_ASSERT(queue_destroy(q) == 0);
}

void test_enqueue_dequeue() {
	fprintf(stderr, "*** TEST e_d ***\n");
    int a = 1, b = 2;
    queue_t q = queue_create();
    queue_enqueue(q, &a);
    queue_enqueue(q, &b);

    int *out;
    queue_dequeue(q, (void **)&out);
    TEST_ASSERT(*out == 1);
    queue_dequeue(q, (void **)&out);
    TEST_ASSERT(*out == 2);
    TEST_ASSERT(queue_length(q) == 0);
    queue_destroy(q);
}

void test_dequeue_empty_fails() {
	fprintf(stderr, "*** TEST d_e_f ***\n");
    queue_t q = queue_create();
    int *out = NULL;
    TEST_ASSERT(queue_dequeue(q, (void **)&out) == -1);
    queue_destroy(q);
}

void test_delete_positions() {
	fprintf(stderr, "*** TEST d_p ***\n");
    int a = 1, b = 2, c = 3;
    queue_t q = queue_create();
    queue_enqueue(q, &a);
    queue_enqueue(q, &b);
    queue_enqueue(q, &c);

    TEST_ASSERT(queue_delete(q, &b) == 0);
    TEST_ASSERT(queue_length(q) == 2);
    TEST_ASSERT(queue_delete(q, &a) == 0); 
    TEST_ASSERT(queue_delete(q, &c) == 0); 
    TEST_ASSERT(queue_length(q) == 0);
    queue_destroy(q);
}

void test_length(void) {
    fprintf(stderr, "*** TEST length ***\n");
    int x = 7;
    queue_t q = queue_create();
    TEST_ASSERT(queue_length(q) == 0);
    queue_enqueue(q, &x);
    TEST_ASSERT(queue_length(q) == 1);
    int *out = NULL;
    TEST_ASSERT(queue_dequeue(q, (void **)&out) == 0);
    TEST_ASSERT(out == &x);
    TEST_ASSERT(queue_length(q) == 0);
    queue_destroy(q);
}

int main(void) {
	test_create();
	test_queue_simple();
	//
	test_create_destroy_empty();
	test_destroy_non_empty();
	test_enqueue_dequeue();
	test_dequeue_empty_fails();
	test_delete_positions();
	test_length();
	return 0;
}
