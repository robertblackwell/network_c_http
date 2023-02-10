
#include <stdio.h>

#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <http_in_c/common/cbuffer.h>
#include <http_in_c/logger.h>
#include <http_in_c/unittest.h>
#include <http_in_c/common/queue.h>

typedef struct Params_s {
	QueueRef qref;
	int max;
	int  id;
	int producer_count;
	int consumer_count;
	bool did_queue_get_full;
} Params, *ParamsRef;

// a steady producer every 1 secnd
void* producer_01(void* data)
{
	int counter = 100;
	ParamsRef pref = data;
	QueueRef qref = pref->qref;
	while(1) {
		for(int i = 0; i < pref->max + 2; i++) {
            int size = Queue_size(qref);
            int cap = Queue_capacity(qref);
            if(cap == size) {
                pref->did_queue_get_full = true;
            }
			Queue_add(qref, counter);
            printf("producer_01 adding %d size: %d capacity: %d \n", counter, size, cap);
			pref->producer_count++;
			printf("producer_01 added %d\n", counter);
			counter += 10;
		}
		sleep(1);
		break;
	}
	Queue_add(qref, -1);
	return NULL;
}

void* consumer_01(void* data)
{
	ParamsRef p = (ParamsRef)data;
	int id = p->id;
    int counter = 100;
	QueueRef qref = p->qref;
	while(1) {
		printf("consumer_%d removing \n", id);
		int value = Queue_remove(qref);
		if(value == -1) {
		    return NULL;
		}
		p->consumer_count++;
		printf("consumer_%d removed %d counter : %d \n", id, value, counter);
		assert(value == counter);
		counter += 10;
	}	
	return NULL;
}

void test()
{
	QueueRef qref = Queue_new();
	pthread_t t1, t2, t3;
	pthread_create(&t1, NULL, &(producer_01), (void*)qref);
	Params  p1={.qref=qref, .id=1000};
	pthread_create(&t2, NULL, &(consumer_01), (void*)&p1);
//	Params  p2={.qref=qref, .id=2000};
//	pthread_create(&t3, NULL, &(consumer_01), (void*)&p2);

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	printf("leaving test\n");
}
/**
 * tests that producer suspends and starts again after queue is full
 */
int test_pc_01()
{
    QueueRef qref = Queue_new_with_capacity(5);
    pthread_t t1, t2, t3;
    Params  p1={.qref=qref, .id=1000, .max=7, .producer_count=0, .consumer_count=0, .did_queue_get_full=false};
    pthread_create(&t1, NULL, &(producer_01), (void*)&p1);
    pthread_create(&t2, NULL, &(consumer_01), (void*)&p1);
    Params  p2={.qref=qref, .id=1000, .max=7, .producer_count=0, .consumer_count=0, .did_queue_get_full=false};
//	pthread_create(&t3, NULL, &(consumer_01), (void*)&p2);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    UT_EQUAL_INT(p1.consumer_count, p1.producer_count);
    UT_TRUE(p1.did_queue_get_full);
    printf("leaving test\n");
    return 0;
}
/**
 * Test simple add/remove
 */
int test_simple()
{
    QueueRef queue = Queue_new_with_capacity(5);
    Queue_add(queue, 100);
    Queue_add(queue, 200);
    Queue_add(queue, 300);
    Queue_add(queue, 400);
    Queue_add(queue, 500);
    int f0 = Queue_remove(queue);
    UT_EQUAL_INT(f0, 100);
    int f1 = Queue_remove(queue);
    UT_EQUAL_INT(f1, 200);
    int f2 = Queue_remove(queue);
    UT_EQUAL_INT(f2, 300);
    int f3 = Queue_remove(queue);
    UT_EQUAL_INT(f3, 400);
    int f4 = Queue_remove(queue);
    UT_EQUAL_INT(f4, 500);
    return 0;
}
int main()
{
    UT_ADD(test_simple);
	UT_ADD(test_pc_01);
	UT_RUN();
}