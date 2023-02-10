
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <http_in_c/unittest.h>
#include <http_in_c//runloop/runloop.h>
#include <http_in_c/runloop/rl_internal.h>
/**
 * This module tests a new memory allocation scheme that uses a static cache of objects
 * of a given type and a free list to manage allocation
 * @return
 */
#define RTOR_MAX_TIMER 4096
typedef struct RtorTimerAllocUnit_s RtorTimerAllocUnit, *RtorTimerAllocUnitPtr;
struct RtorTimerAllocUnit_s {
    RtorTimerAllocUnitPtr  next_ptr;
    uint16_t               index;
    bool                   is_allocated;
    RtorTimer              data;
};



typedef struct RtorTimerMemPool_s RtorTimerMemPool, *RtorTimerMemPoolRef;
struct RtorTimerMemPool_s {
    RtorTimerAllocUnitPtr   first_free_ref;
    int                     count;
    RtorTimerAllocUnit      mem[RTOR_MAX_TIMER];
};

RtorTimerMemPool rtor_timer_pool = {.first_free_ref = NULL, .count = RTOR_MAX_TIMER};

void mem_rtor_timer_init()
{
    for(int i = 0; i < rtor_timer_pool.count; i++) {
        rtor_timer_pool.mem[i].index = i;
        rtor_timer_pool.mem[i].next_ptr = rtor_timer_pool.first_free_ref;
        rtor_timer_pool.first_free_ref = &(rtor_timer_pool.mem[i]);
        rtor_timer_pool.mem[i].is_allocated = false;
    }
}
RtorTimerRef memalloc_rtor_timer()
{
    if(rtor_timer_pool.first_free_ref == NULL) {
        return NULL;
    }
    RtorTimerAllocUnitPtr tmp = rtor_timer_pool.first_free_ref;
    rtor_timer_pool.first_free_ref = tmp->next_ptr;
    tmp->is_allocated = true;
    return &(tmp->data);
}

void memfree_rtor_timer(RtorTimerRef timer_ref)
{
    void* p = (void*) timer_ref;
    void* zero_ptr = 0;
    RtorTimerAllocUnitPtr allounit_ptr = (RtorTimerAllocUnitPtr) zero_ptr;
    long offset = (long)&(allounit_ptr->data);
    void* start_ptr = p - offset;
    RtorTimerAllocUnitPtr tau_ptr = (RtorTimerAllocUnitPtr)start_ptr;
    RtorTimerAllocUnitPtr tmp = rtor_timer_pool.first_free_ref;
    rtor_timer_pool.first_free_ref = tau_ptr;
    tau_ptr->next_ptr = tmp;
    tau_ptr->is_allocated = false;
    printf("hello there\n");
}
bool mem_rtor_timer_allfree()
{
    for(int i = 0; i < rtor_timer_pool.count; i++) {
        if(rtor_timer_pool.mem[i].is_allocated) {
            return false;
        }
    }
    return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Generic allocation
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define GENERIC_TAG "GENTAG\0"
typedef struct GenericAllocUnit_s {
    uint64_t               tag;
    RtorTimerAllocUnitPtr  next_ptr;
    uint16_t               index;
    bool                   is_allocated;
    void*                  data;
} GenericAllocUnit;
typedef struct GenericMemPool_s {
    GenericAllocUnit*       first_free_ref;
    void*                   end_ptr;
    uint16_t                item_sizeof;
    uint16_t                item_alloc_size;
    int                     count;
    GenericAllocUnit*       mem;
} GenericMemPool;


GenericMemPool* mem_pool_init(int how_many, int size_of_item)
{
    // calc size to allocate for each item
    int sz1 = sizeof(GenericAllocUnit);
    int sz2 = sizeof(void*);
    int part1 = (((sizeof(GenericAllocUnit) - sizeof(void*) + 7) >> 3) << 3);
    int part2 = (((size_of_item + 7) >> 3) << 3);
    int final_item_size = part1 + part2;
    // calculate the size fo the contiguous pool
    int psiz1 = sizeof(GenericMemPool);
    int psiz2 = sizeof(uint64_t);
    int p_part1 = (((sizeof(GenericMemPool) - sizeof(uint64_t) + 7) >> 3) << 3);
    int p_part2 = final_item_size * how_many;
    int final_pool_size = p_part1 + p_part2;
    GenericMemPool* pool = malloc(final_pool_size);
    pool->end_ptr = (pool + final_pool_size - 1);
    void* endptr;
    // fill the pool memory with 'X' and calculate the address of the end byte
    for(int j = 0; j < final_pool_size; j++) {
        char* cc = pool;
        cc[j] = 'X';
        endptr = &cc[j];
        char zz = cc[j];
    }
    pool->first_free_ref = NULL;
    pool->end_ptr = endptr;
    pool->item_sizeof = size_of_item;
    pool->item_alloc_size = final_item_size;
    pool->count = how_many;
    GenericAllocUnit* au_ptr = &(pool->mem);
    for(int i = 0; i < how_many; i++) {
        memset((void*) au_ptr, 0, pool->item_alloc_size);
        int ll = strlen(GENERIC_TAG);
        strncpy((char*) &(au_ptr->tag), GENERIC_TAG, strlen(GENERIC_TAG));
        bool test = au_ptr >= pool->end_ptr;
        void* tmp_end_ptr = (((char*)pool) + final_pool_size - 1);
        bool test2 = au_ptr >= (tmp_end_ptr);
        au_ptr->next_ptr = pool->first_free_ref;
        pool->first_free_ref = au_ptr;
        au_ptr->is_allocated = false;
        au_ptr->index = i;
        au_ptr = (GenericAllocUnit*)(((char*)au_ptr) + pool->item_alloc_size);

    }
}
int test_2()
{
    GenericMemPool* mp = mem_pool_init(100, sizeof(RtorTimer));
    return 0;
}


int test_1()
{
    int sz1 = sizeof(RtorTimer);
    int sz2 = sizeof(RtorTimerAllocUnit);
    int sz3 = sizeof(RtorTimerMemPool);
    RtorTimerMemPool* pool = &(rtor_timer_pool);
    mem_rtor_timer_init();
    UT_TRUE(mem_rtor_timer_allfree());
    RtorTimerAllocUnitPtr f1 = (pool->first_free_ref);
    RtorTimerRef tref = memalloc_rtor_timer();
    UT_EQUAL_PTR(&(f1->data), tref)
    UT_TRUE(!mem_rtor_timer_allfree());
    RtorTimerAllocUnitPtr x1 = &rtor_timer_pool.mem[RTOR_MAX_TIMER - 1];
    RtorTimerAllocUnitPtr x2 = &rtor_timer_pool.mem[RTOR_MAX_TIMER - 2];
    RtorTimerRef y1 = &(rtor_timer_pool.mem[RTOR_MAX_TIMER - 1].data);
    UT_EQUAL_PTR(x2, pool->first_free_ref);
    ReactorRef rx = rtor_reactor_new();
    rtor_timer_init(tref, rx);
    memfree_rtor_timer(tref);
    UT_TRUE(mem_rtor_timer_allfree());
    return 0;
}
int main()
{
//    UT_ADD(test_2);
    UT_ADD(test_1);
    int rc = UT_RUN();
    return rc;
}