#ifndef c_ceg_queue_h
#define c_ceg_queue_h
//
// This is a queue of integers (socket fd) with a fixed max size.
// Consumers wait on empty 
// producers wait on full
//
struct Queue_s;
typedef struct Queue_s Queue, *QueueRef;

QueueRef Queue_new();
void Queue_free(QueueRef* q_p);
int Queue_remove(QueueRef q);
void Queue_add(QueueRef q, int port);


#endif