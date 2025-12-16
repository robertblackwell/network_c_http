
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rbl/check_tag.h>
#include <rbl/unittest.h>
#include <allocators/object_pool/alloc_object_pool.h>
#include <allocators/object_pool/alloc_object_pool_internal.h>

/**
* Simple demo of why Rust memory lifetime stuff might be valuable.
*
* This program runs without apparent problems but run under valgrind displays a evident memory corruption
* which can also be easily seen from the code.
*/

typedef struct Example_s {
    char    key[10];
    char    value[100];  
} Example;


char* a_bug_here(Example* ex_p)
{
    return &(ex_p->value[0]);
}
typedef struct Test_s {
    RBL_DECLARE_TAG;
    union {
        long timer[10];
    };
    RBL_DECLARE_END_TAG;
} TestType, *TestTypeRef;


int test01()
{
    int nbr_objs = 30;
    ObjectPool* pool = v2_object_pool_create(48, nbr_objs);
    for(int i = 0; i < nbr_objs; i++) {
        MemoryBlock* bp = op_block_at(pool, i);
        op_block_fill_object(pool, bp, 'z', v2_object_pool_obj_size(pool));
    }
    for(int i = 0; i < nbr_objs; i++) {
        void* bp = op_block_at(pool, i);
        OBJECT_POOL_BLOCK_VERIFY(pool, bp);
        op_block_object_fill_check(pool, bp, 'z');
    }
    printf("Hello");
    return 0;
}
void fill_object_mem(ObjectPool* pool, void* objp, void* blkp, char* output_buffer)
{
    if(output_buffer == NULL) {
        output_buffer = (char*)objp;
    }
    size_t n = pool->obj_size;
    size_t m = snprintf(output_buffer, n, "%p %p", objp, blkp);
    char*  pp = ((char*)output_buffer) + n - m - 1;
    size_t m2 = snprintf(pp, n, "%p %p", objp, blkp);
}
int test02()
{
    int nbr_objs = 30;
    int obj_size = 48;
    ObjectPool* pool = v2_object_pool_create(obj_size, nbr_objs);
    void* ptr[nbr_objs];
    for(int i = 0; i < nbr_objs; i++) {
        void* first_free_blk = pool->free_list.head;
        void* tmp = v2_object_pool_allocate(pool, __FILE__, __LINE__);
        UT_TRUE(tmp != NULL);
        memset(tmp, 'X', obj_size);
        void* blkp = op_block_blkptr_from_objptr(pool, tmp);
        void* first_allocated = pool->allocated_list.head;
        UT_TRUE(first_free_blk == blkp);
        UT_TRUE(first_allocated == blkp);

        void* objp = op_block_get_object_start(pool, blkp);
        UT_TRUE(objp == tmp);
        // snprintf(objp, 30, "%p %p", objp, blkp);
        fill_object_mem(pool, objp, blkp, NULL);
        ptr[i] = tmp;
    }

    size_t howmany = v2_object_pool_number_in_use(pool);
    UT_TRUE(howmany == nbr_objs);
    for(int i = 0; i < nbr_objs; i++) {
        void* tmp = ptr[i];
        MemoryBlock* bp = op_block_blkptr_from_objptr(pool, tmp);
        OBJECT_POOL_BLOCK_VERIFY(pool, bp);
        char buf[pool->obj_size];
        fill_object_mem(pool, tmp, bp, buf);
        int x = strcmp(buf, (char*)tmp);
        UT_TRUE((x == 0));
        UT_TRUE(tmp != NULL);
        MemoryBlock* f1 = op_block_list_find(&(pool->allocated_list), bp);
        MemoryBlock* f2 = op_block_list_find(&(pool->free_list), bp);
        UT_TRUE(f1 == bp)
        UT_TRUE(f2 == NULL)
        v2_object_pool_deallocate(pool, tmp, __FILE__, __LINE__);
        MemoryBlock* f3 = op_block_list_find(&(pool->allocated_list), bp);
        MemoryBlock* f4 = op_block_list_find(&(pool->free_list), bp);
        UT_TRUE(f4 == bp)
        UT_TRUE(f3 == NULL)
    }

    for(int i = 0; i < nbr_objs; i++) {
        void* tmp = v2_object_pool_allocate(pool, __FILE__, __LINE__);
        UT_TRUE(tmp != NULL);
        ptr[i] = tmp;
    }
    return 0;
}
//
// int test_freelist()
// {
//     FreeListRef fl = freelist_new(10);
//     uint16_t* pbuffer = (uint16_t*)fl->buffer;
//     for(int i = 0; i < 10; i++) {
//         uint16_t* p = (pbuffer + i);
//         uint16_t k = pbuffer[i];
//         UT_TRUE(k == i);
//         printf("i: %d k: %d\n", i, k);
//     }
//     UT_TRUE(fl->rdix == 0)
//     UT_TRUE(fl->wrix == 10)
//     UT_TRUE(fl->count == 10)
//     printf("hello\n");
//     return 0;
// }
int main()
{
    // UT_ADD(test_freelist);
    UT_ADD(test01);
    UT_ADD(test02);
    int rc = UT_RUN();
    return rc;
}