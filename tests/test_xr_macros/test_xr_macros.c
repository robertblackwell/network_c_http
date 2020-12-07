#define _GNU_SOURCE
#define XR_PRINTF_ENABLE
#define XR_TRACE_ENABLE
#include <c_http/xr/types.h>
#include <c_http/unittest.h>

int test_macros()
{
    XR_TRACE("this is test %s ", "thisisarg");
    return 0;
}
int main()
{
    UT_ADD(test_macros);
    int rc = UT_RUN();
    return rc;
}