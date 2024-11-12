#ifndef c_http_xr_wtable_h
#define c_http_xr_wtable_h
#include <http_in_c/runloop/runloop.h>

struct FdTable_s;

typedef struct FdTable_s FdTable, *FdTableRef;

FdTableRef FdTable_new();
void FdTable_free(FdTableRef this);
void FdTable_insert(FdTableRef this, WatcherRef wref, int fd);
void FdTable_remove(FdTableRef this, int fd);
WatcherRef FdTable_lookup(FdTableRef this,int fd);
int FdTable_iterator(FdTableRef this);
int FdTable_next_iterator(FdTableRef this, int iter);
uint64_t FdTable_size(FdTableRef this);
#endif