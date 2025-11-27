
#include <stdio.h>
#include <string.h>
#include <rbl/unittest.h>
#include <src/common/arena.h>
#include <common/arena_internal.h>
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
    return 0;
}
int test_size_round_up()
{
    ArenaPtr arena = arena_create(4*1024);
    UT_TRUE(arena != NULL);
    void* p1 = arena_alloc(arena, 91);
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
    // the -8 is to allow for the size_t field at the start of each
    // allocated block
    ArenaPtr arena = arena_create(4*1024);
    UT_TRUE(arena != NULL);
    void* p1 = arena_alloc(arena, 1024-8);
    fill(p1, 'a', 1024-8);
    UT_TRUE(arena->begin->next_block_ptr == NULL)
    void* p2 = arena_alloc(arena, 1024-8);
    fill(p2, 'b', 1024-8);
    UT_TRUE(arena->begin->next_block_ptr == NULL)
    void* p3 = arena_alloc(arena, 1024-8);
    fill(p3, 'c', 1024-8);
    UT_TRUE(arena->begin->next_block_ptr == NULL)
    void* p4 = arena_alloc(arena, 1024-8);
    fill(p4, 'd', 1024-8);
    UT_TRUE(arena->begin->next_block_ptr == NULL)
    void* p5 = arena_alloc(arena, 1024-8);
    fill(p5, 'd', 1024-8);
    UT_TRUE(arena->begin->next_block_ptr != NULL)
    return 0;
}
int test_add_block_02()
{
    ArenaPtr arena = arena_create(4*1024);
    UT_TRUE(arena != NULL);
    void* p1 = arena_alloc(arena, 1024-8);
    fill(p1, 'a', 1024-8);
    // arena has only 1 block
    UT_TRUE(arena->begin->next_block_ptr == NULL)
    UT_TRUE(arena->begin == arena->end)
    void* p2 = arena_alloc(arena, 1024-8);
    fill(p2, 'b', 1024-8);
    // arena still has only 1 block
    UT_TRUE(arena->begin->next_block_ptr == NULL)
    UT_TRUE(arena->begin == arena->end)

    void* p3 = arena_alloc(arena, 3*(1024-8));
    fill(p3, 'c', 3*(1024-8));
    // arena now has 2 blocks and p3 is in the second block
    UT_TRUE(arena->begin->next_block_ptr != NULL)
    UT_TRUE(arena->begin != arena->end)
    UT_TRUE(arena->begin->next_block_ptr->next_block_ptr == NULL)
    UT_TRUE(arena->begin->next_block_ptr == arena->end)
    void* contaning_blk = arena_find_block(arena, p3);
    UT_TRUE(contaning_blk != NULL);
    UT_TRUE(contaning_blk == arena->end);

    void* p4 = arena_alloc(arena, 1024-8);
    fill(p4, 'd', 1024-8);
    // p4 is in the first block
    void* contaning_blk2 = arena_find_block(arena, p4);
    UT_TRUE(contaning_blk2 != NULL);
    UT_TRUE(contaning_blk2 == arena->begin);

    void* p5 = arena_alloc(arena, 1024-8);
    fill(p5, 'd', 1024-8);
    void* contaning_blk3 = arena_find_block(arena, p5);
    UT_TRUE(contaning_blk3 != NULL);
    UT_TRUE(contaning_blk3 == arena->begin);

    void* p6 = arena_realloc(arena, p5, 3*(1024-8));
    check_fill(p6, 'd', 0, 1024-8);
    check_fill(p6, 'z', 1024-8, 3*(1024-8));

    return 0;
}
int main()
{
    UT_ADD(test_simple);
    UT_ADD(test_size_round_up);
    UT_ADD(test_add_block);
    UT_ADD(test_add_block_02);
    int rc = UT_RUN();
    return rc;
}