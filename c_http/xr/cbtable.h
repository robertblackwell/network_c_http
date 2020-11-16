#ifndef reactor_htable_h
#define reactor_htable_h
#include <c_http/reactor/reactor.h>

typedef struct xr_watcher_s {
    xr_watcher_type type;
    union {
        xr_timer_watcher_ref  tw;
        xr_queue_watcher_ref  qw;
        xr_socket_watcher_ref sw;
    }
};

typedef struct {
    Callback 	callback;
    void 		*arg;
} CallbackData;

CallbackData* CbData_new(Callback callback, void *arg);
void CbData_free(CallbackData* this);

struct CbTable_s;

typedef struct CbTable_s CbTable, *CbTableRef;

CbTableRef CbTable_new();
void CbTable_free(CbTableRef this);
void CbTable_insert(CbTableRef this, Callback callback, void* arg, int fd);
void CbTable_remove(CbTableRef this, int fd);
CallbackData* CbTable_lookup(CbTableRef this,int fd);
int CbTable_iterator(CbTableRef this);
int CbTable_next_iterator(CbTableRef this, int iter);

#endif