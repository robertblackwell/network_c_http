#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <rbl/unittest.h>
#include <stdlib.h>
#include <rbl/logger.h>
#include <common/vec.h>


int test_01()
{
    Vec_p vtmp = vec_new(10);
    for (int i = 0; i < 12; i++) {
        vec_append(vtmp, (void*)(long)i);
    }
    for (int i = 0; i < 12; i++) {
        const int tmp = (int)(long)vec_at(vtmp, i);
        UT_TRUE(tmp == i);
    }
    printf("all  done");
    return 0;
}

int main(void)
{
    UT_ADD(test_01);
    int r = UT_RUN();
    return r;
}