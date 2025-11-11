
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rbl/check_tag.h>
#include <rbl/unittest.h>
#include <common/object_pool.h>
#include <common//object_pool_internal.h>

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
    ObjectPool* pool = object_pool_create(48, nbr_objs);
    for(int i = 0; i < nbr_objs; i++) {
        void* bp = block_at(pool, i);
        blk_set_tag(pool, block_at(pool, i), "ABCDEFGH");
        blk_set_index(pool, block_at(pool, i), (uint16_t)i);
        blk_set_end_tag(pool, block_at(pool, i), "JKLMNOPQ");
        blk_fill_object(pool, block_at(pool, i), 'z', object_pool_obj_size(pool));
    }
    for(int i = 0; i < nbr_objs; i++) {
        void* bp = block_at(pool, i);
        blk_check_tag(pool, bp, "ABCDEFGH");
        blk_check_end_tag(pool, bp, "JKLMNOPQ");
        UT_TRUE(blk_get_index(pool, bp) == i);
    }
    printf("Hello");
    return 0;
}
int test02()
{
    int nbr_objs = 30;
    int obj_size = 48;
    ObjectPool* pool = object_pool_create(obj_size, nbr_objs);
    void* ptr[nbr_objs];
    for(int i = 0; i < nbr_objs; i++) {
        void* tmp = object_pool_allocate(pool);
        UT_TRUE(tmp != NULL);
        memset(tmp, 'X', obj_size);
        void* blkp = blkptr_from_objptr(pool, tmp);
        uint16_t ix = blk_get_index(pool, blkp);
        UT_TRUE(i == ix)
        void* blkp_2nd = block_at(pool, ix);
        UT_TRUE(blkp == blkp_2nd)
        void* objp = blk_get_object_start(pool, blkp);
        UT_TRUE(objp == tmp)
        blk_set_tag(pool, blkp, "ABCDEFGH");
        blk_set_end_tag(pool, blkp, "JKLMNOPQ");
        ptr[i] = tmp;
        if(i == (nbr_objs-1)) {
            UT_TRUE(pool->free_list_ptr->rdix == 0)
            UT_TRUE(pool->free_list_ptr->wrix == 0)
            UT_TRUE(pool->free_list_ptr->count == 0)
        } else {
            UT_TRUE(pool->free_list_ptr->rdix == i + 1)
            UT_TRUE(pool->free_list_ptr->wrix == nbr_objs)
            UT_TRUE(pool->free_list_ptr->count == nbr_objs - (i + 1))
        }
    }
    UT_TRUE(pool->free_list_ptr->rdix == 0)
    UT_TRUE(pool->free_list_ptr->wrix == 0)
    UT_TRUE(pool->free_list_ptr->count == 0)

    size_t howmany = object_pool_number_in_use(pool);
    UT_TRUE(howmany == nbr_objs);
    for(int i = 0; i < nbr_objs; i++) {
        void* tmp = ptr[i];
        UT_TRUE(tmp != NULL);
        object_pool_deallocate(pool, tmp);
        UT_TRUE(pool->free_list_ptr->rdix == 0)
        UT_TRUE(pool->free_list_ptr->wrix == i+1)
        UT_TRUE(pool->free_list_ptr->count == (i+1))
    }
    UT_TRUE(pool->free_list_ptr->rdix == 0)
    UT_TRUE(pool->free_list_ptr->wrix == nbr_objs)
    UT_TRUE(pool->free_list_ptr->count == nbr_objs)

    for(int i = 0; i < nbr_objs; i++) {
        void* tmp = object_pool_allocate(pool);
        UT_TRUE(tmp != NULL);
        ptr[i] = tmp;
        if(i == (nbr_objs-1)) {
            UT_TRUE(pool->free_list_ptr->rdix == 0)
            UT_TRUE(pool->free_list_ptr->wrix == 0)
            UT_TRUE(pool->free_list_ptr->count == 0)
        } else {
            UT_TRUE(pool->free_list_ptr->rdix == i + 1)
            UT_TRUE(pool->free_list_ptr->wrix == nbr_objs)
            UT_TRUE(pool->free_list_ptr->count == nbr_objs - (i + 1))
        }
    }
    UT_TRUE(pool->free_list_ptr->rdix == 0)
    UT_TRUE(pool->free_list_ptr->wrix == 0)
    UT_TRUE(pool->free_list_ptr->count == 0)
    return 0;
}

int test_freelist()
{
    FreeListRef fl = freelist_new(10);
    uint16_t* pbuffer = (uint16_t*)fl->buffer;
    for(int i = 0; i < 10; i++) {
        uint16_t* p = (pbuffer + i);
        uint16_t k = pbuffer[i];
        UT_TRUE(k == i);
        printf("i: %d k: %d\n", i, k);
    }
    UT_TRUE(fl->rdix == 0)
    UT_TRUE(fl->wrix == 10)
    UT_TRUE(fl->count == 10)
    printf("hello\n");
    return 0;
}
int main()
{
    UT_ADD(test_freelist);
    UT_ADD(test01);
    UT_ADD(test02);
    int rc = UT_RUN();
    return rc;
}