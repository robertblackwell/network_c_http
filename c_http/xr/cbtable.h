#ifndef c_http_xr_wtable_h
#define c_http_xr_wtable_h
#include <c_http/xr/watcher.h>

struct CbTable_s;

typedef struct CbTable_s CbTable, *CbTableRef;

CbTableRef CbTable_new();
void CbTable_free(CbTableRef this);
void CbTable_insert(CbTableRef this, XrWatcherRef wref, int fd);
void CbTable_remove(CbTableRef this, int fd);
XrWatcherRef CbTable_lookup(CbTableRef this,int fd);
int CbTable_iterator(CbTableRef this);
int CbTable_next_iterator(CbTableRef this, int iter);
uint64_t CbTable_size(CbTableRef this);
#endif