#ifndef c_http_xr_evfd_queue_h
#define c_http_xr_evfd_queue_h

/**
 * This is an inter thread queue that signals "something added to the queue" in a manner that
 * will cause an epoll event.
 *
 * One way of doing this is what is called "the two pipe trick"
 * I have also attempted to achive the desired result with a single FD
 * the producer writes a single byte to the FD
 * The consumer uses epoll to wait for the FD to become readable.
 */

typedef struct EvfdQueue_s EvfdQueue, *EvfdQueueRef;

EvfdQueueRef Evfdq_new();
void  Evfdq_free(EvfdQueueRef this);
int   Evfdq_readfd(EvfdQueueRef this);
void  Evfdq_add(EvfdQueueRef this, void* item);
void* Evfdq_remove(EvfdQueueRef this);

#endif