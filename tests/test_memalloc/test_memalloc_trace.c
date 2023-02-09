#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <http_in_c/unittest.h>
#include <http_in_c//runloop/runloop.h>
#include <http_in_c/runloop/rl_internal.h>
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Allocation functions that use malloc but provide trace of who allocated and deallocated
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define TRACE_TAG "TRACE\0"
typedef struct TraceAllocUnit_s TraceAllocUnit, * TraceAlocUnitPtr;
struct TraceAllocUnit_s {
    union {
        uint64_t tag64;
        char     tagchar[8];
    };
    char                    file_name[16];
    int                     line_nbr;
    TraceAllocUnit*         next_ptr;
    void*                  data;
};
typedef struct TraceMemPool_s {
    TraceAllocUnit*         first_used_ref;
    void*                   end_ptr;
    uint16_t                item_sizeof;
    uint16_t                item_alloc_size;
    int                     count;
} TraceMemPool;


TraceMemPool* rtor_tracemem_pool_init(int how_many, int size_of_item)
{
    // calc size to allocate for each item
    int sz1 = sizeof(TraceAllocUnit);
    int sz2 = sizeof(void*);
    int part1 = (((sizeof(TraceAllocUnit) - sizeof(void*) + 7) >> 3) << 3);
    int part2 = (((size_of_item + 7) >> 3) << 3);
    int final_item_size = part1 + part2;
    // calculate the size fo the contiguous pool
//    int psiz1 = sizeof(GenericMemPool);
//    int psiz2 = sizeof(uint64_t);
//    int p_part1 = (((sizeof(GenericMemPool) - sizeof(uint64_t) + 7) >> 3) << 3);
//    int p_part2 = final_item_size * how_many;
//    int final_pool_size = p_part1 + p_part2;
    TraceMemPool* pool = malloc(sizeof(GenericMemPool));
    pool->first_used_ref = NULL;
    pool->item_sizeof = size_of_item;
    pool->item_alloc_size = final_item_size;
    pool->count = how_many;
}

void* rtor_tracemem_alloc(int size, char* file, int line)
{
    // calculate the size of the allocation unit to allocate
    int sz1 = sizeof(TraceAllocUnit);
    int sz2 = sizeof(void*);
    int part1 = (((sizeof(TraceAllocUnit) - sizeof(void*) + 7) >> 3) << 3);
    int part2 = (((size + 7) >> 3) << 3);
    int final_item_size = part1 + part2;
    TraceAllocUnit* tmp = malloc()
    if(pool->first_free_ref == NULL) {
        return NULL;
    }
    GenericAlocUnitPtr tmp = pool->first_free_ref;
    pool->first_free_ref = tmp->next_ptr;
    tmp->is_allocated = true;
    return (void*)&(tmp->data);
}
bool mem_pool_checktag(GenericAllocUnit* alloc_unit_ptr)
{
    if( (strlen(alloc_unit_ptr->tagchar) == strlen(GENERIC_TAG))
        || (0 == (strcmp(alloc_unit_ptr->tagchar, GENERIC_TAG))))
        return true;
    return false;
}
void mem_pool_free(GenericMemPool* pool, void* ptr)
{
    void* p = ptr;
    void* zero_ptr = 0;
    GenericAllocUnit* allounit_ptr = (GenericAllocUnit*) zero_ptr;
    long offset = (long)&(allounit_ptr->data);
    void* start_ptr = p - offset;
    GenericAllocUnit* au_ptr = (GenericAllocUnit*)start_ptr;
    int ix = au_ptr->index;
    assert(ix > 0 && ix < pool->count);
    void* addr1 = &((char*)(pool->mem) + ix * pool->item_alloc_size;
    void* addr2 = au_ptr;
    assert(addr1 == addr2);
    GenericAllocUnit* tmp = pool->first_free_ref;
    pool->first_free_ref = au_ptr;
    au_ptr->next_ptr = tmp;
    au_ptr->is_allocated = false;
    printf("hello there\n");

}
int test_2()
{
    GenericMemPool* mp = mem_pool_init(100, sizeof(RtorTimer));
    return 0;
}


int main()
{
    UT_ADD(test_2);
    int rc = UT_RUN();
    return rc;
}