#ifndef c_http_xr_evfd_queue_h
#define c_http_xr_evfd_queue_h

typedef struct EvfdQueue_s EvfdQueue, *EvfdQueueRef;

EvfdQueueRef Evfdq_new();
void Evfdq_free(EvfdQueueRef this);
int Evfdq_readfd(EvfdQueueRef this);
void Evfdq_add(EvfdQueueRef this, void* item);
void* Evfdq_remove(EvfdQueueRef this);

#endif