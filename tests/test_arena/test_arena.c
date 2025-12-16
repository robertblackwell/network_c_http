
#include <stdio.h>
#include <string.h>
#include <rbl/unittest.h>
#include <rbl/check_tag.h>
#include <allocators/arena/arena.h>
#include <allocators/arena/arena_internal.h>
#include <src/common/iobuffer.h>
struct TestBlock
{
    size_t a;
    uint8_t mem[];
};
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

int test_simple()
{
    printf("XXXXXXXXXXXHello world from buffer test \n");
    size_t x = sizeof(uint8_t);
    size_t y = sizeof(void*);
    size_t z = sizeof(intptr_t);
    size_t tagsize = rbl_tag_size();
    UT_TRUE(y == z)
    UT_TRUE(x == 1)
    uint8_t xa[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    intptr_t xb[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    uint8_t* ya = &(xa[0]);
    intptr_t* yb = &(xb[0]);
    uint8_t* za = ya;
    za += 1;
    UT_TRUE(((size_t)za == (((size_t)ya) + 1)))
    intptr_t* zb = yb;
    zb += 1;
    UT_TRUE(((size_t)zb == (((size_t)yb) + 8)))

    size_t ss1 = sizeof(AllocatedMemory);
    size_t ss11 = offsetof(AllocatedMemory, mem);
    size_t ss2 = sizeof(AllocatedMemory) + sizeof(RedZone);
    size_t ss3 = sizeof(AllocatedMemory) + sizeof(RedZone) + offsetof(AllocatedMemory, next_allocated_memory_ptr);
    size_t ss4 = arena_allocated_memory_overhead();
    return 0;
}
int test_size_round_up()
{
    ArenaPtr arena = arena_create(4*1024);
    UT_TRUE(arena != NULL);
    void* p1 = ARENA_ALLOC(arena, 91);
    UT_TRUE(p1 != NULL);
    char* pchar = (char*)p1;
    fill(p1, 'a', 91);
    for(int i = 0; i < 91; i++) {
        UT_TRUE(*(pchar+i) == 'a')
    }
    for(int i = 91; i < 96; i++) {
        UT_TRUE(*(pchar+i) == 'z')
    }
    return 0;
}
int test_add_block()
{
    // the overhead is to allow for the size_t field at the start of each
    // allocated block
    size_t overhead = arena_allocated_memory_overhead();
    ArenaPtr arena = arena_create(4*1024);
    UT_TRUE(arena != NULL);
    size_t ss = sizeof(AllocatedMemory);
    void* p1 = ARENA_ALLOC(arena, 1024-overhead);
    void* q = ((char*)&(arena->begin->mem)) + arena->begin->mem_next_byte_index;
    *(char*)q = 'X';
    arena_verify_allocation(arena, p1);
    fill(p1, 'a', 1024-overhead);
    arena_verify_allocation(arena, p1);
    UT_TRUE(arena->begin->next_block_ptr == NULL)
    void* p2 = ARENA_ALLOC(arena, 1024-overhead);
    arena_verify_allocation(arena, p1);
    arena_verify_allocation(arena, p2);
    fill(p2, 'b', 1024-overhead);
    UT_TRUE(arena->begin->next_block_ptr == NULL)
    void* p3 = ARENA_ALLOC(arena, 1024-overhead);
    fill(p3, 'c', 1024-overhead);
    UT_TRUE(arena->begin->next_block_ptr == NULL)
    void* p4 = ARENA_ALLOC(arena, 1024-overhead);
    fill(p4, 'd', 1024-overhead);
    UT_TRUE(arena->begin->next_block_ptr == NULL)
    void* p5 = ARENA_ALLOC(arena, 1024-overhead);
    fill(p5, 'd', 1024-overhead);
    UT_TRUE(arena->begin->next_block_ptr != NULL)
    arena_verify_allocation(arena, p1);
    arena_verify_allocation(arena, p2);
    arena_verify_allocation(arena, p3);
    arena_verify_allocation(arena, p4);
    arena_verify_allocation(arena, p5);
    return 0;
}
int test_add_block_02()
{
    size_t overhead = arena_allocated_memory_overhead();
    ArenaPtr arena = arena_create(4*1024);
    UT_TRUE(arena != NULL);
    void* p1 = ARENA_ALLOC(arena, 1024-overhead);
    fill(p1, 'a', 1024-overhead);
    // arena has only 1 block
    UT_TRUE(arena->begin->next_block_ptr == NULL)
    UT_TRUE(arena->begin == arena->end)
    void* p2 = ARENA_ALLOC(arena, 1024-overhead);
    fill(p2, 'b', 1024-overhead);
    // arena still has only 1 block
    UT_TRUE(arena->begin->next_block_ptr == NULL)
    UT_TRUE(arena->begin == arena->end)

    void* p3 = ARENA_ALLOC(arena, 3*(1024-overhead));
    fill(p3, 'c', 3*(1024-overhead));
    // arena now has 2 blocks and p3 is in the second block
    UT_TRUE(arena->begin->next_block_ptr != NULL)
    UT_TRUE(arena->begin != arena->end)
    UT_TRUE(arena->begin->next_block_ptr->next_block_ptr == NULL)
    UT_TRUE(arena->begin->next_block_ptr == arena->end)
    void* contaning_blk = arena_find_block(arena, p3);
    UT_TRUE(contaning_blk != NULL);
    UT_TRUE(contaning_blk == arena->end);

    void* p4 = ARENA_ALLOC(arena, 1024-overhead);
    fill(p4, 'd', 1024-overhead);
    // p4 is in the first block
    void* contaning_blk2 = arena_find_block(arena, p4);
    UT_TRUE(contaning_blk2 != NULL);
    UT_TRUE(contaning_blk2 == arena->begin);

    void* p5 = ARENA_ALLOC(arena, 1024-overhead);
    fill(p5, 'd', 1024-overhead);
    void* contaning_blk3 = arena_find_block(arena, p5);
    UT_TRUE(contaning_blk3 != NULL);
    UT_TRUE(contaning_blk3 == arena->begin);

    void* p6 = ARENA_REALLOC(arena, p5, 3*(1024-overhead));
    check_fill(p6, 'd', 0, 1024-overhead);
    check_fill(p6, 'z', 1024-overhead, 3*(1024-overhead));

    return 0;
}
int test_correct_size()
{
    size_t overhead = arena_allocated_memory_overhead();
    ArenaPtr arena = arena_create(1024);
    UT_TRUE(arena != NULL);
    void* p1 = ARENA_ALLOC(arena, 512-overhead);
    size_t fs = arena_block_free_space(arena->begin);
    size_t rs = arena_block_require_freespace(512-overhead);
    UT_TRUE(rs == fs)
    bool can_satisfy_1 = arena_block_can_satisfy_alloc(arena->begin, 512);
    UT_TRUE((!can_satisfy_1))
    bool can_satisfy_12 = arena_block_can_satisfy_alloc(arena->begin, 512-overhead+1);
    UT_TRUE((!can_satisfy_12))
    bool can_satisfy_2 = arena_block_can_satisfy_alloc(arena->begin, 512-overhead);
    UT_TRUE((can_satisfy_2))
    return 0;
}
int main()
{
    UT_ADD(test_simple);
    UT_ADD(test_size_round_up);
    UT_ADD(test_add_block);
    UT_ADD(test_add_block_02);
    UT_ADD(test_correct_size);
    int rc = UT_RUN();
    return rc;
}