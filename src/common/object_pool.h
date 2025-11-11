
#ifndef H_runloop_object_pool_allocator_H
#define H_runloop_object_pool_allocator_H
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <rbl/check_tag.h>

typedef struct FreeList_s FreeList, *FreeListRef;
typedef struct MemorySlab_s MemorySlab, *MemorySlabRef;
typedef struct ObjectPool_s ObjectPool, *ObjectPoolRef;

/**
 * create a new ObjectPool
 * @param obj_size - the size of the memory needed for a single object - must be 64 bit aligned
 * @param obj_count - the max number of objects this pool is to provide
 */
ObjectPoolRef object_pool_create(int obj_size, int obj_count);
/**
 * allocate an object from the pool
 */
void* object_pool_allocate(ObjectPoolRef et);
/**
 * return an object to the pool
 */
void object_pool_deallocate(ObjectPoolRef op, void* objptr);
/**
 * returns true if the pool has outstanding (that is allocated but not returned)
 * objects
 */
bool object_pool_has_outstanding_objects(ObjectPoolRef et);
/**
 * Return the number of objects currently allocated from the pool
 */
size_t object_pool_number_in_use(ObjectPoolRef et);
/**
 * destroy the pool and return memory resources to the system
 */
void object_pool_destroy(ObjectPoolRef et);

#endif