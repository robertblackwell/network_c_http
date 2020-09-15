#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <c_eg/unittest.h>
#include <c_eg/buffer/contig_buffer.h>
#include <c_eg/logger.h>

int test_something_1()
{
	printf("this is in the testcase %s \n", __FUNCTION__);
	return 0;
}
int test_something_2()
{
    printf("this is in the testcase %s \n", __FUNCTION__);
    return 0;
}
int test_something_3()
{
    printf("this is in the testcase %s \n", __FUNCTION__);
    UT_EQUAL_INT(1, 2);
    return 0;
}
int test_something_4()
{
    printf("this is in the testcase %s \n", __FUNCTION__);
    return 0;
}

int test_1(void)
{
    printf("this is " RED("red") "!\n");

    // a somewhat more complex ...
    printf("this is " BLUE("%s") "!\n","blue");

    return 0;
}


void test_f()
{
    LOG_FMT("%s %d", "this is a string parameter", 42);
}

int test_2()
{
    test_f();
    UT_EQUAL_INT(1, 2);
}

int main()
{
	UT_ADD(test_something_1);
    UT_ADD(test_something_3);
    UT_ADD(test_something_2);
    UT_ADD(test_something_4);
	UT_RUN();
}