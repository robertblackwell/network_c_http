#define _GNU_SOURCE
#define ENABLE_LOG
#include <c_http/async/types.h>
#include <c_http/unittest.h>
#include <c_http/logger.h>

int test_macros()
{
    LOG_FMT("this is test %s ", "thisisarg");
    return 0;
}
int main()
{
    UT_ADD(test_macros);
    int rc = UT_RUN();
    return rc;
}