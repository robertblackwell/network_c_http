#ifndef c_ceg_queue_h
#define c_ceg_queue_h
//
// This is a queue of integers (socket fd) with a fixed max size.
// Consumers wait on empty 
// producers wait on full
//
struct Queue_s;
typedef struct Queue_s Queue;

Queue* Queue_new();
void Queue_free(Queue** q_p);
int Queue_remove(Queue* q);
void Queue_add(Queue* q, int port);


#endif