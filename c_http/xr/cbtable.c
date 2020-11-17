#include <c_http/xr/cbtable.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <c_http/xr/watcher.h>

#define CBTABLE_MAX 4096

struct CbTable_s {
	XrWatcher* entries[CBTABLE_MAX];
};

typedef struct CbTable_s CbTable, *CbTableRef;

void CbTable_init(CbTableRef this)
{
	for(int i = 0; i < CBTABLE_MAX; i++) {
		this->entries[i] = NULL;
	}
}
CbTableRef CbTable_new()
{
	CbTableRef tmp = malloc(sizeof(CbTable));
	CbTable_init(tmp);
	return tmp;
}
void CbTable_free(CbTableRef this)
{
	for(int i = 0; i < CBTABLE_MAX; i++) {
		if (this->entries[i] != NULL) {
		    CbTable_remove(this, i);
		}
	}
	free((void*)this);
}
void CbTable_insert(CbTableRef this, XrWatcherRef watcher, int fd)
{
	assert(this->entries[fd] == NULL);
	this->entries[fd] = watcher;
}
void CbTable_remove(CbTableRef this, int fd)
{
	assert(this->entries[fd] != NULL);
	XrWatcherRef wr = (this->entries[fd]);
	wr->free(wr);
	this->entries[fd] = NULL;
}
XrWatcherRef CbTable_lookup(CbTableRef this, int fd)
{
	assert(this->entries[fd] != NULL);
	return 	(this->entries[fd]);

}
int iterator(CbTableRef this)
{
    for(int i = 0; i < CBTABLE_MAX; i++) {
        if (this->entries[i] != NULL) {
            return i;
        }
    }
    return -1;
}
int next_iterator(CbTableRef this, int iter)
{
    assert(iter+1 < CBTABLE_MAX);
    for(int i = iter+1; i < CBTABLE_MAX; i++) {
        if (this->entries[i] != NULL) {
            return i;
        }
    }
    return -1;
}
