
#include <pthread.h>
#include <unistd.h>
#include <c_eg/buffer/contig_buffer.h>
#include <c_eg/logger.h>
#include <c_eg/queue.h>

typedef struct Params_s {
	Queue* qref;
	int  id;
} Params, *ParamsRef;

// a steady producer every 1 secnd
void* producer_01(void* data)
{
	int counter = 100;
	Queue* qref = (Queue*)data;
	while(1) {
		for(int i = 0; i < 12; i++) {
			printf("producer_01 adding %d\n", counter);
			Queue_add(qref, counter);
			printf("producer_01 added %d\n", counter);
			counter += 10;
		}
		sleep(1);
	}
	return NULL;
}

void* consumer_01(void* data)
{
	ParamsRef p = (ParamsRef)data;
	int id = p->id;

	Queue* qref = p->qref;
	while(1) {
		printf("consumer_%d removing \n", id);
		int value = Queue_remove(qref);
		printf("consumer_%d removed %d\n", id, value);
	}	
	return NULL;
}

void test()
{
	Queue* qref = Queue_new();
	pthread_t t1, t2, t3;
	pthread_create(&t1, NULL, &(producer_01), (void*)qref);
	Params  p1={.qref=qref, .id=1000};
	pthread_create(&t2, NULL, &(consumer_01), (void*)&p1);
	Params  p2={.qref=qref, .id=2000};
	pthread_create(&t3, NULL, &(consumer_01), (void*)&p2);

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
}

int main()
{
	test();
}