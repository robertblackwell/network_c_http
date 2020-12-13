#include <c_http/runloop/fdtable.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <c_http/runloop/watcher.h>

#define CBTABLE_MAX 4096

struct FdTable_s {
    uint64_t count;
	Watcher* entries[CBTABLE_MAX];
};

typedef struct FdTable_s FdTable, *FdTableRef;

void FdTable_init(FdTableRef this)
{
    this->count = 0;
	for(int i = 0; i < CBTABLE_MAX; i++) {
		this->entries[i] = NULL;
	}
}
FdTableRef FdTable_new()
{
	FdTableRef tmp = malloc(sizeof(FdTable));
	FdTable_init(tmp);
	return tmp;
}
void FdTable_free(FdTableRef this)
{
	for(int i = 0; i < CBTABLE_MAX; i++) {
		if (this->entries[i] != NULL) {
		    FdTable_remove(this, i);
		}
	}
	free((void*)this);
}
void FdTable_insert(FdTableRef this, WatcherRef watcher, int fd)
{
	assert(this->entries[fd] == NULL);
	this->entries[fd] = watcher;
	this->count++;
}
void FdTable_remove(FdTableRef this, int fd)
{
	assert(this->entries[fd] != NULL);
	WatcherRef wr = (this->entries[fd]);
	wr->free(wr);
	this->entries[fd] = NULL;
	this->count--;
}
WatcherRef FdTable_lookup(FdTableRef this, int fd)
{
	assert(this->entries[fd] != NULL);
	return 	(this->entries[fd]);

}
int FdTable_iterator(FdTableRef this)
{
    for(int i = 0; i < CBTABLE_MAX; i++) {
        if (this->entries[i] != NULL) {
            return i;
        }
    }
    return -1;
}
int FdTable_next_iterator(FdTableRef this, int iter)
{
    assert(iter+1 < CBTABLE_MAX);
    for(int i = iter+1; i < CBTABLE_MAX; i++) {
        if (this->entries[i] != NULL) {
            return i;
        }
    }
    return -1;
}
uint64_t FdTable_size(FdTableRef this)
{
    return this->count;
}