#include <c_http/reactor/cbtable.h>
#include <c_http/reactor/reactor.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

#define CBTABLE_MAX 4096

CallbackData* CbData_new(Callback callback, void *arg);
void CbData_free(CallbackData* this);

struct CbTable_s {
	CallbackData* entries[CBTABLE_MAX]; 
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
			CbData_free(this->entries[i]);
		}
	}
	free((void*)this);
}
void CbTable_insert(CbTableRef this, Callback callback, void* arg, int fd)
{
	assert(this->entries[fd] == NULL);
	this->entries[fd] = CbData_new(callback, arg);
}
void CbTable_remove(CbTableRef this, int fd)
{
	assert(this->entries[fd] != NULL);
	CbData_free(this->entries[fd]);
	this->entries[fd] = NULL;
}
CallbackData* CbTable_lookup(CbTableRef this, int fd)
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
