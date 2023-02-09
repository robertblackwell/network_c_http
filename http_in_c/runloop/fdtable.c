
#include <http_in_c/runloop/rl_internal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>


struct FdTable_s {
    DECLARE_TAG;
    uint64_t        count;
	RtorWatcher*    entries[RTOR_FDTABLE_MAX];
    char            end_tag[TAG_LENGTH];
};

typedef struct FdTable_s FdTable, *FdTableRef;

void FdTable_init(FdTableRef this)
{
    SET_TAG(FdTable_TAG, this);
    SET_TAG_FIELD(FdTable_TAG, this, end_tag);
    this->count = 0;
	for(int i = 0; i < RTOR_FDTABLE_MAX; i++) {
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
    CHECK_TAG(FdTable_TAG, this);
    CHECK_TAG_FIELD(FdTable_TAG, this, end_tag)
	for(int i = 0; i < RTOR_FDTABLE_MAX; i++) {
		if (this->entries[i] != NULL) {
		    FdTable_remove(this, i);
		}
	}
	free((void*)this);
}
void FdTable_insert(FdTableRef this, RtorWatcherRef watcher, int fd)
{
    CHECK_TAG(FdTable_TAG, this);
    CHECK_TAG_FIELD(FdTable_TAG, this, end_tag)
	assert(this->entries[fd] == NULL);
	this->entries[fd] = watcher;
	this->count++;
}
/**
 *  This function does not free() the item being removed from the fdList
 */
void FdTable_remove(FdTableRef athis, int fd)
{
    CHECK_TAG(FdTable_TAG, athis);
    CHECK_TAG_FIELD(FdTable_TAG, athis, end_tag)
	assert(athis->entries[fd] != NULL);
	RtorWatcherRef wr = (athis->entries[fd]);
//	wr->free(wr);
	athis->entries[fd] = NULL;
	athis->count--;
}
RtorWatcherRef FdTable_lookup(FdTableRef this, int fd)
{
    CHECK_TAG(FdTable_TAG, this);
    CHECK_TAG_FIELD(FdTable_TAG, this, end_tag)
	assert(this->entries[fd] != NULL);
	return 	(this->entries[fd]);

}
int FdTable_iterator(FdTableRef this)
{
    CHECK_TAG(FdTable_TAG, this);
    CHECK_TAG_FIELD(FdTable_TAG, this, end_tag)
    for(int i = 0; i < RTOR_FDTABLE_MAX; i++) {
        if (this->entries[i] != NULL) {
            return i;
        }
    }
    return -1;
}
int FdTable_next_iterator(FdTableRef this, int iter)
{
    CHECK_TAG(FdTable_TAG, this);
    CHECK_TAG_FIELD(FdTable_TAG, this, end_tag)
    assert(iter+1 < RTOR_FDTABLE_MAX);
    for(int i = iter+1; i < RTOR_FDTABLE_MAX; i++) {
        if (this->entries[i] != NULL) {
            return i;
        }
    }
    return -1;
}
uint64_t FdTable_size(FdTableRef this)
{
    CHECK_TAG(FdTable_TAG, this);
    CHECK_TAG_FIELD(FdTable_TAG, this, end_tag)
    return this->count;
}