
#include <http_in_c/runloop/rl_internal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>


struct FdTable_s {
    RBL_DECLARE_TAG;
    uint64_t        count;
	RunloopWatcher*    entries[runloop_FDTABLE_MAX];
    RBL_DECLARE_END_TAG;
};

typedef struct FdTable_s FdTable, *FdTableRef;

void FdTable_init(FdTableRef this)
{
    RBL_SET_TAG(FdTable_TAG, this);
    RBL_SET_END_TAG(FdTable_TAG, this);
    this->count = 0;
	for(int i = 0; i < runloop_FDTABLE_MAX; i++) {
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
    RBL_CHECK_TAG(FdTable_TAG, this);
    RBL_CHECK_END_TAG(FdTable_TAG, this)
	for(int i = 0; i < runloop_FDTABLE_MAX; i++) {
		if (this->entries[i] != NULL) {
		    FdTable_remove(this, i);
		}
	}
	free((void*)this);
}
void FdTable_insert(FdTableRef this, RunloopWatcherRef watcher, int fd)
{
    RBL_CHECK_TAG(FdTable_TAG, this);
    RBL_CHECK_END_TAG(FdTable_TAG, this)
	assert(this->entries[fd] == NULL);
	this->entries[fd] = watcher;
	this->count++;
}
/**
 *  This function does not free() the item being removed from the fdList
 */
void FdTable_remove(FdTableRef athis, int fd)
{
    RBL_CHECK_TAG(FdTable_TAG, athis);
    RBL_CHECK_END_TAG(FdTable_TAG, athis)
	assert(athis->entries[fd] != NULL);
	RunloopWatcherRef wr = (athis->entries[fd]);
//	wr->free(wr);
	athis->entries[fd] = NULL;
	athis->count--;
}
RunloopWatcherRef FdTable_lookup(FdTableRef this, int fd)
{
    RBL_CHECK_TAG(FdTable_TAG, this);
    RBL_CHECK_END_TAG(FdTable_TAG, this)
    if(this->entries[fd] == NULL) {
        assert(this->entries[fd] != NULL);
    }
	return 	(this->entries[fd]);

}
int FdTable_iterator(FdTableRef this)
{
    RBL_CHECK_TAG(FdTable_TAG, this);
    RBL_CHECK_END_TAG(FdTable_TAG, this)
    for(int i = 0; i < runloop_FDTABLE_MAX; i++) {
        if (this->entries[i] != NULL) {
            return i;
        }
    }
    return -1;
}
int FdTable_next_iterator(FdTableRef this, int iter)
{
    RBL_CHECK_TAG(FdTable_TAG, this);
    RBL_CHECK_END_TAG(FdTable_TAG, this)
    assert(iter+1 < runloop_FDTABLE_MAX);
    for(int i = iter+1; i < runloop_FDTABLE_MAX; i++) {
        if (this->entries[i] != NULL) {
            return i;
        }
    }
    return -1;
}
uint64_t FdTable_size(FdTableRef this)
{
    RBL_CHECK_TAG(FdTable_TAG, this);
    RBL_CHECK_END_TAG(FdTable_TAG, this)
    return this->count;
}