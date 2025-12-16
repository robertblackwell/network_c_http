
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <rbl/unittest.h>
#include <allocators/alloc_threadlocal/mblock.h>
#include <src/common/iobuffer.h>

#include <allocators/alloc_threadlocal/intrusive_list.h>
#include <allocators/alloc_threadlocal/tl_allocator.h>

static void fill(void* p, char ch, size_t n)
{
    while(n-- > 0) {
        ((uint8_t*)p)[n] = ch;
    }
}
static void check_fill(void* p, char ch, size_t begin_n, size_t end_n)
{
    size_t n = begin_n;
    while(n < end_n) {
        char* q = &((char*)p)[n];
        assert(((uint8_t*)p)[n] == ch);
        n++;
    }
}
int test_pointer_as_array()
{
    // a struct that points at an array of pointers
    typedef struct Memory_s
    {
        size_t count;
        size_t max_count;
        void** mem_ptr;
    } Memory;
    Memory* m = malloc(sizeof(Memory));
    m->count = 0;
    m->max_count = 100;
    m->mem_ptr = malloc(m->max_count * sizeof(void*));
    void* p = malloc(5 * sizeof(void*));
    m->mem_ptr[m->count++] = p;
    return 0;
}
int test_simple()
{
    printf("XXXXXXXXXXXHello world from buffer test \n");
    MBlock* blk = memblock_new(120);
    void* userptr = memblock_user_ptr(blk);
    fill(userptr, 'a', blk->free_space_size);
    UT_TRUE(blk->mem[0] == 'a')
    UT_TRUE(blk->mem[blk->free_space_size-1] == 'a')
    char* p = Block_Free_TAG;
    UT_TRUE((char)(blk->mem[blk->free_space_size]) == *p++)
    UT_TRUE((char)(blk->mem[blk->free_space_size+1]) == *p++)
    return 0;
}
int test_split()
{
    MBlock* blk = memblock_new(160);
    void* userptr = memblock_user_ptr(blk);
    void* next1 = memblock_after(blk);
    size_t k1 = (intptr_t)next1 - (intptr_t)blk;
    size_t blk_size_original = memblock_size(blk->free_space_size);
    size_t original_free_space = blk->free_space_size;
    UT_TRUE(k1 == blk_size_original)
    UT_TRUE(blk->free_space_size == 160)
    fill(userptr, 'a', blk->free_space_size);
    MBlock* next = memblock_after(blk);

    size_t x1 = ((50 / 16)+1)*16;
    size_t x2 = memblock_size(x1);
    MBlock* left_over =memblock_split(blk, 50);
    // blk contains the requested allocation
    // leftover - is the unused part of the split
    void* next2 = memblock_after(blk);
    size_t k2 = (intptr_t)next2 - (intptr_t)blk;
    // void* next_3 = memblock_after(blk);
    // size_t k_3 = (intptr_t)next_3 - (intptr_t)blk;
    // size_t x_3 = memblock_size(blk->free_space_size);
    UT_TRUE(k2 == x2) // 2 different ways to calculate the size of the block blk
    UT_TRUE(left_over->free_space_size == 40)
    void* userptr2 = memblock_user_ptr(blk);
    fill(userptr2, 'x', blk->free_space_size);

    size_t x3 = memblock_size(blk->free_space_size);
    void* next3 = memblock_after(blk);
    size_t k3 = (intptr_t)next3 - (intptr_t)blk;
    UT_TRUE(k2 == x2)

    size_t n1 = blk_size_original;
    size_t n2 = memblock_size(left_over->free_space_size);
    size_t n3 = memblock_size(blk->free_space_size);
    bool b = (n1 = n2 + n3);
    UT_TRUE(b)
    bool aj = memblock_adjacent(blk, left_over);
    UT_TRUE(aj)
    MBlock* merged = memblock_merge(blk, left_over);
    UT_TRUE(merged == blk)
    UT_TRUE(merged->free_space_size == original_free_space)
    void* merged_userptr = memblock_user_ptr(merged);
    fill(merged_userptr, 'W', blk->free_space_size);
    UT_TRUE(merged->mem[0] == 'W')
    UT_TRUE(merged->mem[blk->free_space_size-1] == 'W')
    char* p = Block_Free_TAG;
    UT_TRUE((char)(merged->mem[blk->free_space_size]) == *p++)
    UT_TRUE((char)(merged->mem[blk->free_space_size+1]) == *p++)
    printf("hello world from buffer test \n");
    return 0;
}
int test_freelist_01()
{
    MBlockList* freelist = tl_intrusive_list_new();
    tl_intrusive_list_add(freelist, memblock_new(256));
    UT_TRUE(freelist->head->free_space_size == 256)
    UT_TRUE(freelist->tail->free_space_size == 256)
    UT_TRUE(freelist->head->forward == NULL)
    UT_TRUE(freelist->head->backward == NULL)
    tl_intrusive_list_add(freelist, memblock_new(128));
    UT_TRUE(freelist->head->free_space_size == 256)
    UT_TRUE(freelist->head->forward->free_space_size == 128)
    UT_TRUE(freelist->head->forward->forward == NULL)
    tl_intrusive_list_add(freelist, memblock_new(1024));
    UT_TRUE(freelist->head->free_space_size == 1024)
    UT_TRUE(freelist->head->forward->free_space_size == 256)
    UT_TRUE(freelist->head->forward->forward->free_space_size == 128)
    UT_TRUE(freelist->head->forward->forward->forward == NULL)
    UT_TRUE(freelist->head->forward->forward->backward == freelist->head->forward)
    UT_TRUE(freelist->head->forward->backward == freelist->head)
    UT_TRUE(freelist->head->backward == NULL)
    UT_TRUE(freelist->tail->forward == NULL)
    UT_TRUE(freelist->tail->free_space_size  == 128)
    tl_intrusive_list_add(freelist, memblock_new(512));
    UT_TRUE(freelist->head->free_space_size == 1024)
    UT_TRUE(freelist->head->forward->free_space_size == 512)
    UT_TRUE(freelist->head->forward->forward->free_space_size == 256)
    UT_TRUE(freelist->head->forward->forward->forward->free_space_size == 128)
    UT_TRUE(freelist->head->forward->forward->forward == freelist->tail)
    UT_TRUE(freelist->head->forward->forward->forward->forward == NULL)
    UT_TRUE(freelist->head->forward->forward->backward == freelist->head->forward)
    UT_TRUE(freelist->head->forward->backward == freelist->head)
    UT_TRUE(freelist->head->backward == NULL)
    UT_TRUE(freelist->tail->forward == NULL)
    UT_TRUE(freelist->tail->free_space_size  == 128)
    MBlock* nb = tl_intrusive_list_find_space(freelist, 136);
    UT_TRUE(nb->free_space_size >= 136)
    tl_intrusive_list_remove(freelist, nb);

    return 0;
}
#define NBR_ALLOCATIONS 400
int test_tl_allocate()
{
    srand(time(NULL));
    Tlocal_Allocator* a = tl_allocator_create();
    void* user_pointers[NBR_ALLOCATIONS];
    MBlock* block_pointers[NBR_ALLOCATIONS];
    size_t n = 0;
    for (int i = 0; i < NBR_ALLOCATIONS; i++) {
        size_t k = 128 + (size_t)rand() % 259;
        void* p = tl_allocator_alloc(a, k);
        if(p == NULL) {
            break;
        }
        MBlock* b = memblock_from_userptr(p);
        user_pointers[i] = p;
        block_pointers[i] = b;
        memblock_check_tags(b);
        n = i;
    }
    for(int i = 0; i < n+1; i++) {
        void* p = user_pointers[i];
        MBlock* b = block_pointers[i];
        tl_allocator_free(a, p);
    }
    MBlock* b = a->free_list->head;
    size_t bsize = memblock_size(b->free_space_size);
    void* p = memblock_user_ptr(b);
    fill(p, 'H', b->free_space_size);
    MBlock* nx = memblock_after(b);
    UT_TRUE(a->allocated_list->head == NULL);
    tl_allocator_reset(a);
    return 0;
}
int main()
{
    UT_ADD(test_simple);
    UT_ADD(test_split);
    UT_ADD(test_freelist_01);
    UT_ADD(test_tl_allocate);
    int rc = UT_RUN();
    return rc;
}