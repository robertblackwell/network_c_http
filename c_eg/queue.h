#ifndef c_ceg_queue_h
#define c_ceg_queue_h
#include <stdint.h>
//
// This is a queue of integers (socket fd) with a fixed max size.
// Consumers wait on empty 
// producers wait on full
//
typedef int32_t SocketFD;
struct Queue_s;
typedef struct Queue_s Queue, *QueueRef;

QueueRef Queue_new();
void Queue_free(QueueRef* q_p);
SocketFD Queue_remove(QueueRef q);
void Queue_add(QueueRef q, SocketFD socket);


#endif